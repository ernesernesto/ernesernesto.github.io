+++
title = 'Building Simple Audio Visualization with Unity ECS Part III — ECS'
date = 2018-08-06T12:40:06+07:00
toc = false
+++

This is the last part from the three part series of creating a simple audio visualizer to learn ECS. The [first part]({{< relref "BuildingSimpleAudioVisualizationUnityECSPartI" >}}) we talked about how to make things work with MonoBehavior way. The [second part]({{< relref "BuildingSimpleAudioVisualizationUnityECSPartII" >}}) we change the MonoBehavior into a Jobified Unity Job System. Today we’re going to walk to our end goal, transforming our Audio Visualization into ECS form.

The promise of the new ECS system is the performance gain. Usually before moving from one coding principles to another, we have to list all the problems that we could get when changing into the new paradigm. Is the performance gain worth the hassle? Is it worth the time investing things to change the codebase into this new ECS things? But in games, almost always we must pursue the performance gain, because the experience of playing a games smoothly would always beat codework.

The ECS guarantees linear data layout when iterating entities in chunks, this is the critical performance gains that is promised by ECS, before delving deeply into ECS we need to clear out few things.

ECS have three core concept, Entity, Component and the System. Entity is basically an ID, you can add component or remove component into it. The component itself is classes which contains data with no behavior, it is used purely for data. Then come the system which told to do a specific things. Sounds confused? If you’re creating a system to scale all cube entities then all you’ve to do is query all entities in the world that have “cube” id, then for each entity, we’re going to scale the object like we used to do.

There are a couple of changes that we need to do in our code, but first we must identify which one is entity, our component data, and the systems that are going to work on them.

Our entity in this case is the cube, component data is the origin, then for the systems we need a system to spawn all the cube, process the audio data, then change the scale of the cubes, at least three system.

```c#
public class BootupECSJob
{
    public static MeshInstanceRenderer cubeLook;
    public static Settings settings;

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
    public static void Init()
    {
        EntityManager entityManager = World.Active.GetOrCreateManager<EntityManager>();

        GameObject proto = GameObject.Find("CubePrototype");
        if(proto != null)
        {
            cubeLook = proto.GetComponent<MeshInstanceRendererComponent>().Value;
            Object.Destroy(proto);

            GameObject go = GameObject.Find("Settings");
            settings = go.GetComponent<Settings>();

            int channelCount = World.Active.GetOrCreateManager<SpectrumSystem>().Init(settings);
            World.Active.GetOrCreateManager<SpawnCubeSystem>().Init(settings, 
                                                                    channelCount,
                                                                    entityManager,
                                                                    cubeLook);

            World.Active.GetExistingManager<TransformSystem>().Enabled = false;
        }
    }
}

public struct Cube : IComponentData {}

public struct Origin : IComponentData 
{
    public float3 Value;
}
```
Our MonoBehavior is now gone, changed into a static `Init` which sole purpose is to initialize our world with the `entityManager`, get the `Settings`, and initialize our systems. `CubePrototype` is used to spawn our cubes that is now added to Unity hierarchy view. `CubePrototype` have `MeshInstanceRenderer` that defines it’s look and Unity built-in `Game Object Entity Script`, this is used so that Unity ECS know that this is an `Game Object Entity` and could be processed on ECS system.

`Cube` is used as an id, every entity that is created with this `Cube` component data is considered a `Cube`. For the `Origin` component data, it is used to store the cube origin value before we offset it.

The process of spawning and arranging the cube spirally has now moved into `SpawnCubeSystem`. Also we disabled the built-in `TransformSystem` on Unity, we will talk about this later.

Here are the `SpawnCubeSystem`

```c#
public class SpawnCubeSystem : ComponentSystem 
{
    Settings settings;
    int totalCubeCount;
    bool isFirstTime;

    public void Init(Settings inSettings, EntityManager entityManager, MeshInstanceRenderer look)
    {
        isFirstTime = true;
        settings = inSettings;

        int totalCubeCount = settings.spectrumSize*2;
        for(int index = 0; 
            index < totalCubeCount; 
            ++index)
        {
            EntityArchetype archetype = entityManager.CreateArchetype(typeof(Cube), 
                                                                      typeof(Origin),
                                                                      typeof(TransformMatrix), 
                                                                      typeof(Position), 
                                                                      typeof(LocalRotation),
                                                                      typeof(Scale));

            Entity cube = entityManager.CreateEntity(archetype);
            entityManager.AddSharedComponentData(cube, look); 
        }
    }

    public struct CubesGroup
    {
        public int Length;
        public ComponentDataArray<Cube> cubes;
        public ComponentDataArray<Position> positions;
        public ComponentDataArray<Origin> origins;
    }

    [Inject] 
    CubesGroup group;

    protected override void OnUpdate()
    {
        if(isFirstTime)
        {
            int spectrumSize = settings.spectrumSize;
            for(int index = 0;
                index < totalCubeCount;
                ++index)
            {
                int groupIndex = index;
                SpiralPos pos = Utils.GetSpiralPos(groupIndex);
                Position position = group.positions[groupIndex];
                position.Value = new float3(pos.x, 0, pos.z);
                group.positions[groupIndex] = position;

                Origin origin = group.origins[groupIndex];
                origin.Value = position.Value;
                group.origins[groupIndex] = origin;
            }
        }

        isFirstTime = false;
    }
}
```

Our `Init` method is used to create the `entityArchetype` and spawning cubes entity from the archetype. As you can see that we create the archetype with the `Cube` component, `Origin`, `TransformMatrix`, `Position`, `LocalRotation` and `Scale`. This means that our `cube` entity that we are going to spawn has all those component data.
A `CubesGroup` is a struct that is used inside `SpawnCubeSystem`, what it does is it’s collecting all Entity which has `Cube`, `Position`, and `Origin`. Then we laid out the cubes positions spirally on the `OnUpdate()` method. We do this one time only so we need to make sure that even though the `OnUpdate` is called each time the system update, it won’t change the position again.

If you want to change the `ComponentData` value, currently the code in the current Unity ECS needs to copy that into another variable first, change it, and then assigning it back. You’ll often see this kind of code in the ECS system.


```c#
Position position = group.positions[groupIndex];
position.Value = new float3(pos.x, 0, pos.z);
group.positions[groupIndex] = position;
```

Unity said it would change it in the future so it’ll be less cumbersome than now. Anyway after we set each positions and origins, we now need create the system to update the scale of the cubes. Below you could see the `SpectrumSystem` snippet

```c#
class SpectrumSystem : JobComponentSystem
{
    Settings settings;

    Channel[] channels;

    public int Init(Settings inSettings)
    {
        settings = inSettings;

        int spectrumSize = settings.spectrumSize;
        channels = new Channel[1];
        channels[0] = new Channel();
        channels[0].Init(0, spectrumSize);

        int result = channels.Length;
        return result;
    }

    struct SpectrumGroup
    {
        public int Length;
        public ComponentDataArray<Position> positions;
        public ComponentDataArray<Origin> origins;
        public ComponentDataArray<Scale> scales;
    }

    [Inject] 
    SpectrumGroup group;

    [ComputeJobOptimization]
    public struct SpectrumJob : IJobParallelFor
    {
        public float maxScale;
        public float dynamics;
        public float epsilon;

        public NativeArray<float> currentSpectrums;
        public NativeArray<float> prevSpectrums;

        public ComponentDataArray<Position> positions;
        public ComponentDataArray<Origin> origins;
        public ComponentDataArray<Scale> scales;

        public void Execute(int index)
        {
            float current = currentSpectrums[index];
            float prev = prevSpectrums[index];

            float val = (dynamics*prev + (1 - dynamics)*current);
            prevSpectrums[index] = val;

            float valAdjusted = val*maxScale;
            float halfHeight = valAdjusted*0.5f;

            Position position = positions[index];
            Origin origin = origins[index];
            Scale scale = scales[index];

            if(val >= epsilon)
            {
                position.Value = new float3(origin.Value.x, halfHeight, origin.Value.z); 
                scale.Value = new float3(1, valAdjusted, 1);
            }
            else
            {
                position.Value = new float3(0, 0, 0); 
                scale.Value = new float3(0, 0, 0);
            }

            scales[index] = scale;
            positions[index] = position;
        }
    }

    protected override JobHandle OnUpdate(JobHandle inputDeps)
    {
        JobHandle jobHandle = CreateJob(true, channel, inputDeps);
        return jobHandle;
    }

    JobHandle CreateJob(bool left, Channel channel, JobHandle inputDeps)
    {
        AudioListener.GetSpectrumData(channel.spectrumBuff, 
                                      channel.channelNumber, 
                                      FFTWindow.BlackmanHarris);
        channel.currentSpectrums.CopyFrom(channel.spectrumBuff);

        SpectrumJob spectrumJob = new SpectrumJob()
        {
            maxScale = settings.maxScale,
            dynamics = settings.dynamics,
            epsilon = settings.epsilon,
            currentSpectrums = channel.currentSpectrums,
            prevSpectrums = channel.prevSpectrums,

            positions = group.positions,
            origins = group.origins,
            scales = group.Length,
        };

        JobHandle handle = spectrumJob.Schedule(group.Length, 1, inputDeps);
        return handle;
    }
}
```

Different from the `SpawnCubeSystem` that is derived from `ComponentSystem`, our `SpectrumSystem` derived from the `JobComponentSystem` since we need to schedule the `SpectrumJob` here. As you notice that the content of the `SpectrumSystem` is not a big difference with the previous Job version. It only added the `SpectrumGroup` to read the component data, everything else is the same.

Now our ECS system is now ready except for one thing, the current unity version that I use don’t have a scale component, so in order to make the transform works, we need to disable the built-in `TransformSystem` and make our own version of `CustomTransformSystem` that take a scale changes into account. You would remember that we disable the `TransformSystem` with


```c#
World.Active.GetExistingManager<TransformSystem>().Enabled = false;
```

The contents of the `CustomTransformSystem` is this

```c#
[System.Serializable]
public struct Scale : IComponentData
{
    public float3 Value;
}
 
[UnityEngine.ExecuteInEditMode]
public class CustomTransformSystem : JobComponentSystem
{
    struct CustomTransformGroup
    {
        public int Length;

        [ReadOnly]
        public ComponentDataArray<Position> Positions;
 
        [ReadOnly]
        public ComponentDataArray<LocalRotation> Rotations;
 
        [ReadOnly]
        public ComponentDataArray<Scale> Scales;
 
        public ComponentDataArray<TransformMatrix> Transforms;
    }
 
    [Inject]
    CustomTransformGroup transformGroup;
 
    [ComputeJobOptimization]
    struct CustomTransformGroupJob : IJobParallelFor
    {
        [ReadOnly]
        public ComponentDataArray<Position> Positions;
 
        [ReadOnly]
        public ComponentDataArray<LocalRotation> Rotations;
 
        [ReadOnly]
        public ComponentDataArray<Scale> Scales;
 
        public ComponentDataArray<TransformMatrix> Transforms;
 
        public void Execute(int i)
        {
            Transforms[i] = new TransformMatrix
            {
                Value = math.mul(math.rottrans(Quaternion.Euler(Rotations[i].Value.value.xyz), Positions[i].Value), math.scale(Scales[i].Value))
            };
        }
    }
 
    protected override JobHandle OnUpdate(JobHandle inputDeps)
    {
        var transformJob = new CustomTransformGroupJob
        {
            Transforms = transformGroup.Transforms,
            Positions = transformGroup.Positions,
            Rotations = transformGroup.Rotations,
            Scales = transformGroup.Scales,
        };
 
        return transformJob.Schedule(transformGroup.Length, 64, inputDeps);
    }
}
```
It sole purpose is to add the scale changes of the component into the `TransformMatrix` ComponentData.

You could see the result video below

{{< youtube S8u0nA8YDNQ >}}

Previously we could only do 4086 sample for two channels, now doing 8192 sample for two channels (16384 data) is no sweat.
Here are the result comparison between MonoBehavior and ECS + Job. I’m testing this on MacBookPro Mid 2015 2.2 GHz Intel Core i7, 16 GB 1600 MHz DDR3.


```
MBP Mid 2015 2.2 GHz Intel Core i7 
MonoBehavior: 8192 cubes = FPS Roughly 45 - 50ish
ECS + Job : 8192 samples x 2 Channels ~ 16384 cubes = FPS 110++
```

The ECS shown on the video is displaying two channels. Again, the code for making two channel displayed is omitted to make things shorter, but what you would do for two channels is making sure you create an Entity with a different component data that is used only for differentiating left and right spectrum. Because the way the `GetSpectrumSample` works that it returns the data linearly, 0 — sampleSize for left channel and 0 — sampleSize for right channel, but the way things processed on the spectrum job is operating things sequentially, ofsetting the index inside the Execute causes some Error on Unity. Or you could try merging the data from left channel and right channel into one contiguous array before it is processed by `SpectrumSystem`, I haven’t tried this approach yet.

And there you have it, the ECS version of a simple Audio Visualizer. We have walk through the process from making a simple MonoBehavior, changing into a JobSystem, and then moving to a ECS one. Full source code for references on [github](https://github.com/ernesernesto/UnityAudioVisualizer).

Thank you for reading! hope this shed some light on how to approach the new Unity ECS system.
