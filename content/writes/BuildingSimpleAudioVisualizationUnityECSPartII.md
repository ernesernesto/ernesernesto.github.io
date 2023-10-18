+++
title = 'Building Simple Audio Visualization with Unity ECS Part II — Job System'
date = 2018-07-30T12:39:57+07:00
toc = false
+++

This is the second part of our journey to learn ECS on unity. If you haven’t read the [previous post]({{< relref "BuildingSimpleAudioVisualizationUnityECSPartI" >}}) I recommend you to take a while to read that in order to know how the audio visualizer work with the old MonoBehavior way.

Today we’re going to take what’s written on the first part and change it into a Jobified Unity Job System.

The Job System, as explained on the Unity pages, take advantages of multiple cores that our machine have. They also guarantee some protection of multi-threading race conditions pitfalls. To make the audio visualizer Jobified, I took inspiration from here . That links have some great examples on how to structure your data, schedule your jobs, and use the results.

What the job system do is breaking your big “process” into a smaller “Jobs”, doing specific task that could be run parallelly. Job receive parameters and operates on data similar to how a method behave. Each job is self contained but it could have a dependency between each other, making other job only launch after a specific job finishes.

Unity Job System code usually have this form

```
1. Create Job, populate the job with data
2. Schedule the Job, you would get a JobHandle that could be used as a dependency for other jobs
3. Finish the Job by calling the Complete on the JobHandle
```

First, we will move the act of processing the audio visualizer data into job

```c#
struct SpectrumJob : IJobParallelForTransform
{
    public float maxScale;
    public float dynamics;
    public float epsilon;
    public NativeArray<float> currentSpectrums;
    public NativeArray<float> prevSpectrums;
    public NativeArray<Vector3> origins;

    public void Execute(int index, TransformAccess transform)
    {
        float current = currentSpectrums[index];
        float prev = prevSpectrums[index];

        float val = (dynamics*prev + (1 - dynamics)*current);
        prevSpectrums[index] = val;

        float valAdjusted = val*maxScale;
        float halfHeight = valAdjusted*0.5f;

        Vector3 origin = origins[index];
        if(val >= epsilon)
        {
            transform.localPosition = new Vector3(origin.x, halfHeight, origin.z);
            transform.localScale = new Vector3(1, valAdjusted, 1);
        }
        else
        {
            transform.localScale = new Vector3(0, 0, 0);
        }
    }
}
```

As you can see all the work of scaling the cube and offseting it’s position now moved into the `SpectrumJob`. There are new `NativeArray` data that is declared inside the Job, those are Unity `NativeContainers` that is introduced in the new job system. The general purpose of those containers is to give added safety system so you’ll get error if two scheduled jobs writing into the same `NativeContainers`. The Job System also prevent accessing data from the main thread, each code executed inside the `Execute()` is considered working in a separated thread from the main thread.

There’s also other rule that needs to be respected when working with the `NativeContainers` with Unity Job Sytem, you could read them here.

We still need old MonoBehavior to create, schedule and complete the `SpectrumJob`, as displayed here.
 
```c#
public class BootupJob : MonoBehaviour
{
    public GameObject cubePrefab;
    public Settings settings;

    public float[] spectrumBuff;
    public Transform[] transforms;
    public NativeArray<float> currentSpectrums;
    public NativeArray<float> prevSpectrums;
    public NativeArray<Vector3> origins;
    public TransformAccessArray transformsAccess;

    SpectrumJob spectrumJob;
    JobHandle jobHandle;

    void Start()
    {
        ...// Omitted
        result.transformsAccess = new TransformAccessArray(result.transforms);
    }

    void Update()
    {
        float dynamics = settings.dynamics;
        float maxScale = settings.maxScale;
        float rotationSpeed = settings.rotationSpeed;
        float epsilon = settings.epsilon;

        gameObject.transform.Rotate(Vector3.up*(rotationSpeed*Time.deltaTime));

        AudioListener.GetSpectrumData(spectrumBuff, 0, FFTWindow.BlackmanHarris);
        currentSpectrums.CopyFrom(spectrumBuff);

        spectrumJob = new SpectrumJob()
        {
            maxScale = inMaxScale,
            dynamics = inDynamics,
            epsilon = inEpsilon,
            currentSpectrums = this.currentSpectrums,
            prevSpectrums = this.prevSpectrums,
            origins = this.origins,
        };

        jobHandle = spectrumJob.Schedule(transformsAccess);
    }
 
    void LateUpdate()
    {
        jobHandle.Complete();
        spectrumJob.prevSpectrums.CopyTo(prevSpectrums);
    }

    void OnDestroy()
    {
        currentSpectrums.Dispose();
        prevSpectrums.Dispose();
        origins.Dispose();
        transformsAccess.Dispose();
    }
}
```

Now on `Update()`, we create the `SpectrumJob`, fill all the data needed for the Job to perform, then schedule it. The new `TransformAccessArray` on the `Start()` method is needed for the `SpectrumJob` to work. Since previously when we create the cube, it’s transform is not laid out linearly on the memory, we need to create a new `TransformAccess` data with the array of transforms for the `SpectrumJob` so it could be worked on. On `LateUpdate`, we ask the `jobHandle` to Complete the job, then we copy the data from `SpectrumJob` into our `prevSpectrums` buffers, this buffer will be feeded into the Job again in the next update.

{{< youtube _1D12g4bwmA>}}

In this video you could see the results with two audio channels, left and right, I omitted the necessary code above for the two channels, but it is basically the same idea but you need to get the spectrum data from the left (0) and the right (1) channel.

Previously on the old MonoBehavior workflow we cannot have this many cube to be displayed smoothly, but changing our code to work with the new JobSystem, our Audio Visualizer now could handle two channels simultaneously!.

With this the job system is done, we’ll move on the next part, we’ll talk about how to move this into our end goal Unity ECS form.

Until next time!
