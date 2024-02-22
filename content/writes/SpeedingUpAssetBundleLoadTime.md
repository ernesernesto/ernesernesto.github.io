+++
title = 'Speeding Up Asset Bundle Load Time'
date = 2023-10-18T09:00:23+07:00
toc = false
+++

## TLDR
- Use Async API `AssetBundle.LoadFromFileAsync`
- Change AssetBundle `BuildAssetBundleOptions` into `ChunkBasedCompression`, optionally use `DisableLoadAssetByFileName` and `DisableLoadAssetByFileNameWithExtension`.
- Adjust Unity Quality Settings `QualitySettings.asyncUploadTimeSlice` and `QualitySettings.asyncUploadBufferSize`

## Checking how things done

In out recent games, we have some issues when loading a lot of Unity Asset Bundles at runtime on the loading screen. It took a couple of seconds for all bundles to finish loading. A bit weird since it's just 10~ish bundle with total contents of ~100mb, it should be done instantly with an iPhone 11 device (Hexa-core 2x2.65 GHz, 4GB Ram, NVME storage).

Asset bundle loading code generally looks like this
```c#
AssetBundle bundle = AssetBundle.LoadFromFile(path);
if(bundle != null)
{
    // Do things related to bundle contents
}

```

At first I tried changing it into the async version of Loading API
```c#

AssetBundleCreateRequest request = AssetBundle.LoadFromFileAsync(path);
yield return request;

AssetBundle bundle = request.assetBundle;
if(bundle != null)
{
    // Do things related to bundle contents
}
```

But from my timings, there are nothing that significantly changed timing wise for loading time.

After digging deeper into Unity docs, there are some flags that might be useful to increase the asset bundle load time, related to how the asset bundles s being [build](https://docs.unity3d.com/ScriptReference/BuildAssetBundleOptions.html). I've been using `BuildAssetBundleOptions.None` and according to docs it build with LZMA compression and requires entire bundle to be decompressed before being used, making load times slower. Changing the build options into `ChunkBasedCompression` LZ4 results in larger bundle files, but the loading speed is twice faster!. This options is the best tradeoff for speed and size of asset bundles.

## Jobs done

Now at this point, we could pat our back and say that the jobs done, but is there anything else that could be improved?

Looking at the build options again, there are some options that could potentially help reducing the load time more, `DisableLoadAssetByFileName` and `DisableLoadAssetByFileNameWithExtension`.

Whenever an asset bundles is build, there are three ways to look up asset, full asset path, asset file name, and asset file name with extension, for example "Assets/Prefabs/Player.prefab", "Player", and "Player.prefab". Those options will disable the asset name lookup table, saving runtime memory and loading performance. 

Since all call that goes to load the assets through the asset bundle is using
```c#
Sprite sprite = bundle.LoadAsset<Sprite>("Player.prefab");
```

I can only use DisableLoadAssetByFileName, using DisableLoadAssetByFileNameWithExtension means I must go into each load asset call and fix the loading code into using the full path, which means a lot more work to do. Will take this in mind for the next game, but need to justify whether the effort and results beats the easiness of the API. 

## But wait there's more!

Before we can change a few more knobs and buttons on unity to speed up things, we need to understand how Unity package things and load those data at runtime, those things are explained in detail [here](https://blog.unity.com/engine-platform/understanding-the-async-upload-pipeline), but basically, you need to change the buffer size and time slice of when unity does the texture/mesh upload to GPU, if you have a lot of textures and meshes on the asset bundles, this will surely help out. You need to tweak `QualitySettings.asyncUploadTimeSlice` and `QualitySettings.asyncUploadBufferSize`, both are in Unity Quality Settings window. The former will adjust the time spent when uploading texture/mesh data on each frme, while the later would adjust the temporary ring buffer size used to upload those data. Generally for larger textures you need larger buffer, tweak and test those values in order to maximize your game loading time.
