+++
title = 'Building Simple Audio Visualization with Unity ECS Part I — MonoBehavior'
date = 2018-07-23T12:25:50+07:00
toc = false
+++

Currently, I’m learning to learn the new ECS and Job system introduced in Unity 2018. Usually the quickest way to learn a new concept quickly is by creating something that could give you an instant gratification, in this case we’re going to build a simple audio visualization.

The new ECS and Job system now promise to give performance boost and a new way to approach problem, letting you focus on the actual problems you are solving which is the data and behavior of your game. This also move things from object oriented design to a data oriented design.

Before you wrap your head around that, it’s always a good idea to make something that you know first, and then slowly transforming it into a new way, in this case the audio visualization will be build with MonoBehavior initially, before slowly transforming it into a job system and full ECS path.

In this post we will briefly make a working program first, let’s sketch out all the things that needs to be done in a simple audio visualization.

```
1. Spawn some block objects, this objects is going to be transformed by the audio data
2. Each update tick, get the spectrum data of the audio
3. Applying audio data to the block objects, scaling them vertically
4. Profit
```

This is the normal MonoBehavior way of doing it in Unity. We’ll go through the code one by one.

```c#
public class BootupMono : MonoBehaviour
{
    public GameObject cubePrefab;
    public Settings settings;

    float[] spectrumBuff;
    float[] prevSpectrumBuff;

    GameObject[] cubes;

    void Start()
    {
        int spectrumSize = settings.spectrumSize;
        spectrumBuff = new float[spectrumSize];
        prevSpectrumBuff = new float[spectrumSize];
        cubes = new GameObject[spectrumSize];

        for(int index = 0;
            index < spectrumSize;
            ++index)
        {
            GameObject obj = Object.Instantiate(cubePrefab);

            SpiralPos pos = Utils.GetSpiralPos(index);
            obj.transform.position = new Vector3(pos.x, 0, pos.z);
            obj.transform.parent = this.transform;
            cubes[index] = obj;
        }
    }

    void Update()
    {
        AudioListener.GetSpectrumData(spectrumBuff, 0, FFTWindow.BlackmanHarris);

        float dynamics = settings.dynamics;
        float maxScale = settings.maxScale;
        float rotationSpeed = settings.rotationSpeed;
        float epsilon = settings.epsilon;
        int spectrumLength = spectrumBuff.Length;

        for(int index = 0; 
            index < spectrumLength; 
            index++)
        {
            float val = (dynamics*prevSpectrumBuff[index] + (1 - dynamics)*spectrumBuff[index]);
            prevSpectrumBuff[index] = val;

            if(val >= epsilon)
            {
                cubes[index].transform.localScale = new Vector3(1, val*maxScale, 1);
                float halfHeight = cubes[index].transform.localScale.y*0.5f;
                Vector3 origin = cubes[index].transform.position;
                cubes[index].transform.position = new Vector3(origin.x, halfHeight, origin.z);
            }
            else
            {
                cubes[index].transform.localScale = new Vector3(0, 0, 0);
            }
        }

        gameObject.transform.Rotate(0, Time.deltaTime*rotationSpeed, 0);
    }
}
```
As you can see on `Start()`, we are going to spawn the cube object, laid it out spirally and initialize the buffers for audio sample data based on the `spectrumSize` that we choose, the buffers and the number of the cube that we spawn needs to be the same since the spectrum data would be the one that change the cube vertical scale.

The meat of the code is on the `Update()` loop. The spectrum data is loaded with `AudioListener.GetSpectrumData()` into the `spectrumBuff` buffer, then we loop through all the spectrum data, each one we transform the scale of the cubes by the `spectrumBuff`.

We got the value of the change by interpolating it between the previous `prevSpectrumBuff` and current buffer `spectrumBuff`, giving it a nice slow changes and not a haphazard changes each update tick.

The keen reader would then ask why do we need to also change the position of the cube after we scale it. This is just a neat trick since the cube prefab that I use have the pivot of the cube in the center of the object. If we scale the cube, then it would scale upwards and downwards, we don’t want to have the cube wiggling up and down each update, so we need to offset the position of the cube upwards, making the cube looked like it only scale upwards, in reality we’re scaling it upwards and downwards, also offsetting it’s vertical position.

Of course you could also fire you favorite 3d modelling software and change the pivot of the cube to be on the ground. But in this case doing it this way is troublesome for me.

The `epsilon` value is used to check whether the audio data is higher than some threshold before scaling it, if not we’re just going to set the scale to (0, 0, 0), which result in “hiding” the cube. On a high frequency, the audio data changes are too small, if we apply the value to scale our cubes, our eyes would have a hard time detecting the changes. Hiding and showing the cube is easier for our eyes to detect it.

{{< youtube 2iVeMVICJ7c >}}

You can see how it looks in this video, bass / lower frequency starts from the center of the spiral, moving outwards to mid and higher frequency on the outer parts of the spiral.

Some quick stats below

```
MBP Mid 2015 2.2 GHz Intel Core i7 
MonoBehavior: 8192 audio samples, FPS Roughly 45 - 50ish
```

Some optimization tips, in order for the cube to have batching enabled, you need to create custom material with the “Enable GPU Instancing” box toggled, if you did that, then on the stats view from your unity main window, you’d see that the number in “Saved by batching” will go up.

We now have a simple working system to visualize audio, on the next chapter, we’ll talk about how to change the old MonoBehavior code into a Jobified system.

Until next time!
