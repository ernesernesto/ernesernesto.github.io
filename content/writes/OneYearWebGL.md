+++
title = 'One Year of Unity WebGL'
date = 2025-06-04T19:47:06+07:00
toc = false
+++

After spending an entire year deep in the trenches of porting Unity games to WebGL, I can confidently say that WebGL is, without question, the most challenging platform Unity supports. What started as an exciting opportunity to bring desktop games to web browsers quickly became a masterclass in patience, problem-solving, and creative workarounds.

If you're considering WebGL development or wondering why your seemingly simple Unity project breaks when you switch the build target to WebGL, this post is for you. Let me walk you through the reality of WebGL development and why it's painful.

## The Harsh Reality: You're Building for 2008 Technology

Here's the uncomfortable truth: **WebGL 2.0 graphics is essentially OpenGL ES 3.0—technology from 2008**. We're talking about the same era as the original iPhone launch. While your desktop game runs on modern graphics APIs, WebGL forces you back to the visual fidelity of a 15-year-old mobile device.

And this isn't changing anytime soon. WebGPU is still in its infancy and will take years to mature. This means accepting that your beautiful, modern Unity project will look and perform like something from the late 2000s when ported to WebGL.

## The Platform Fragmentation Nightmare

### Every Browser is a Different Platform

One of my most painful lessons was discovering that **every browser on every platform is essentially a unique platform**. That smooth 60fps experience in Chrome? It might stutter at 20fps in Firefox. Works great in Firefox on Windows? Completely different story on Firefox Linux. Mobile browsers add another layer of chaos with thermal throttling and vendor-specific quirks.

My testing routine became exhausting:
- **Desktop**: Chrome, Firefox (at minimum)
- **Android**: Samsung Internet, Chrome, Firefox
- **iOS**: Safari (thankfully just Safari)

Creating a WebGL app that works reliably across both desktop AND mobile became one of the most grueling projects I've ever undertaken.

## Render Pipeline Catastrophes

### Don't Just Copy Your Desktop Settings

I learned this the hard way: **you cannot use the same Render Pipeline Asset settings for WebGL that work on PC**. This seems obvious in hindsight, but it's a trap that catches every developer.

Shaders that don't compile at runtime don't just fail gracefully—they can freeze your entire game and potentially crash the browser through console spam. I spent weeks debugging mysterious freezes only to discover they were caused by:
- Texture sizes exceeding WebGL limits  
- Render pipeline features not supported on WebGL

Now I created a completely separate render pipeline default assets and quality settings specifically for WebGL builds.

## Execution Order Chaos

### When Awake Doesn't Come Before OnEnable

WebGL's execution order can be completely different from other platforms, especially when using async/await. I had a perfectly tested player spawning system that worked flawlessly on desktop—until WebGL decided to run a component's OnEnable before another component's Awake, both on the same GameObject.

This shouldn't be possible according to Unity's documented execution order, but WebGL doesn't care about your assumptions. I also discovered that UnityServices.InitializeAsync, which returned instantly on desktop, actually awaited its result on WebGL, revealing edge case bugs I never knew existed.

The solution? **Build and test on WebGL daily**, not as an afterthought. Context is everything when debugging these platform-specific quirks.

## The Loading Time Death Sentence

### 20 Seconds = Dead Game

When did you last wait more than 20 seconds for a web app to load? Never, because you'd close the tab long before then. Yet WebGL builds often have painfully long loading times that are acceptable nowhere except the desktop development environment.

We're conditioned to expect instant web experiences. If Netflix buffers for 5 seconds, we're annoyed. 10 seconds and we reload the page. Your WebGL game gets the same treatment.

**iOS Safari is particularly ruthless**—if your app blocks the main thread for too long during loading (sometimes as little as 15 seconds), it gets killed. Add the ~500MB memory limit on devices with 2GB RAM, and you're forced to severely downscale all assets just to survive the loading process.

Unity 2022 and Unity 6 have made significant improvements to minimal load times, but this remains a critical constraint.

## Single-Threaded Reality Check

### No More Background Processing

WebGL is single-threaded, period. Unity 6 has experimental C++ multithreading, but for practical purposes, every bit of your code runs on one CPU thread. This creates unexpected bottlenecks because many Unity systems use background threads under the hood, especially rendering.

Systems that run smoothly on desktop suddenly become performance killers in the browser. I had to completely redesign data processing systems, breaking operations across multiple frames using coroutines and implementing custom scheduling to prevent frame drops.

## The Browser Sandbox Lockdown

### No File System, No Exceptions

Browsers prioritize security above all else, which means **zero file system access** for your WebGL app. File.WriteAllText()? Doesn't exist in WebGL land. Want users to upload files or save data locally? You'll need JavaScript interop and browser-specific workarounds.

Even PlayerPrefs is limited to 1MB. Want proper file I/O? Hope you're comfortable writing JavaScript interfaces and dealing with browser security policies that vary by vendor.

## Build Pipeline Torture

### Welcome to Unreal Build Times (Although I'm using Unity)
The first time you build for WebGL, prepare for a shock. Expect 10, 20, even 30+ minutes for your first build, depending on optimization settings. Development builds are fastest; optimized builds take forever.

Incremental builds are faster but still take minutes. You'll need to do clean builds regularly when dealing with deployment or loading issues. I kept an empty project specifically for confirming whether issues were project-specific or general WebGL problems.
**Pro tip**: Use the latest Unity editor version for WebGL—I recommend even Preview versions over LTS. WebGL development advances rapidly with each Unity release.

## Memory Management: Every Byte Counts

### WebGL Humbles Memory Usage

Desktop or even mobile development makes you lazy about memory management. WebGL forces discipline. I spent weeks optimizing a simple 3D game that ran perfectly on desktop but crashed browsers within minutes due to memory fragmentation and poor garbage collection handling. Object pooling becomes mandatory—not just for game objects, but for UI elements you never worried about before. Memory profiling becomes a daily ritual rather than an occasional check.

## Why I Still Recommend WebGL (Sometimes)

Despite these challenges, WebGL offers something unique: **truly frictionless distribution**. When a WebGL game works well, it's magical. Players click a link and instantly play—no no installations, no barriers, no downloads (well player still needs to download it for first time, but you get the idea).

The web platform reaches billions of devices and offers analytics capabilities desktop platforms can't match. For casual, social, or educational games, WebGL's reach often justifies the development complexity.

## Hard-Earned Lessons 
### Target for WebGL from Day One

Don't port to WebGL—develop for WebGL from the beginning. Design your architecture around WebGL's limitations rather than retrofitting desktop features. It's relatively easier to target WebGL first and then support mobile rather than the other way around. This single decision will save months of painful refactoring. Plan ahead for addressables, don't use bloated libraries, Unity wasm pipeline is still bad at stripping unecessary code. 

### Embrace the Constraints

WebGL's limitations force better, more efficient code. The games I've shipped on WebGL are leaner and better optimized than their mobile counterparts.

WebGL compatibility issues compound over time. A minor shader problem becomes a major architectural rewrite if ignored. Regular WebGL builds should be part of your core development process, not a deployment afterthought.

### Build Robust Fallback Systems

Plan for failure at every level. Browsers will crash, features will be unavailable, and performance will vary wildly between identical hardware. Your game needs graceful degradation and clear feedback when things go wrong.

## Final Thoughts

Unity WebGL development is harder than Unity's documentation suggests and far more complex than most developers expect. The restrictions continue to be underestimated by developers who treat WebGL as "just another build target." It's not, it's a constrained, sandboxed environment running on 15-year-old graphics technology, where every browser is a unique platform with its own bugs and limitations.

After a year of WebGL, I've learned to respect it, the unique distribution model opens doors that traditional platforms can't.

If you're embarking on WebGL development, acknowledge the challenge ahead. Budget significantly more time than you think you need. Prepare for browser-specific bugs, memory constraints, and performance issues that don't exist anywhere else.

Unity WebGL will humble you.
