+++
title = 'Debugging Memory Allocation'
date = 2018-04-16T12:00:00+07:00
draft = true
toc = false 
+++

Recently I was debugging some particular nasty bugs that happens on memory allocations, the intricate part is, it happens infrequently
My app was crashing on a release build more frequently than in debug builds.

I was doing some manual memory allocations, at the start of the app it will load a saved data from a file into that memory. The app was doing fine until I started to write something to the saved data, then on the next launch it will start to crash intermittently on release build.

I was using the technique explained on Handmadehero, for some of you who didn’t know, Handmadehero is a programming stream in which Casey Muratori aimed to teach a complete game live, no engines, no libraries, created from scratch, one hour a day, and currently with 400+ episodes so far.

So doing the allocations Handmadehero-style, memory allocations happens at the start of the app, preallocated for a specific size.

This gains many benefit in which we don’t need to handle newing and deleting memory every time, and particularly dealing with memory fragmentations, I’ll talk more about this later.

Also in the case that our app reach the memory limit that we set, we could take two approach, either allocating bigger size at the start of the application or use loading unloading mechanism (ex. filling old address of memory with new block of data). By using this technique, we also put a framework in mind to work with by making sure our app doesn’t hit the size limit, since we know that it will crash with out of memory if we don’t have enough room.

### Preallocated Memory Allocations

So the entirety of the bug is this

```c
struct Resource
{
    b32 processing;
    u32 count;
    r64 timeRemaining;
};

struct OwnedResource
{
    u32 count;
    u32 toProduceCount;
    b32 owned;
    b32 pad_;
};

struct AppData
{
    u16 id;
    u16 pad_1;
    u32 pad_2;

    u32 data1;
    u32 data2;
    u32 data3;
    u32 data4;

    OwnedResource* owned;
    Resource* resources;

};

// The Entirety of The Bug
{
    MemoryArena arena = {};
    size_t memSize = Megabytes(32);
    void* memory = malloc(memSize);
    arena.size = memSize;
    arena.base = (u8*)memory;
    arena.used = 0;

    AppData* appData = pushStruct(&arena, AppData);
    appData->owned = pushArray(&arena, ResourceType_Count, OwnedResource);
    appData->resources = pushArray(&arena, ResourceType_Count, Resource);

    FILE* dataFile = fopen("file.dat", "rb");
    if(dataFile)
    {
        fread(appData, sizeof(AppData), 1, dataFile);

        // Crash happens here
        fread(appData->owned, sizeof(OwnedResource)*ResourceType_Count, 1, dataFile);
        fread(appData->resources, sizeof(Resource)*ResourceType_Count, 1, dataFile);
        fclose(dataFile);
    }
}
```
 As you can see I aligned all of my structs so it will be 8 byte aligned with a pad, if you don’t aligned those data, serializing back and forth is erroneous since memory works with alignment.
`u32` `b32` `r64` is an alias for unsigned/bool/real followed with its size.

At first the app allocated a linearly contiguous 32mb sized memory, then it starts to `pushStruct` and `pushArrays` into the memory arena. With this the memory is always contiguous and laids out linearly inside the arena, not distributed all over the place in the case where we do a `new` each time we need a data stored on heap.

Crash happens more frequently on release builds. On debug build, the only information that I got when crashing was the address of appData->owned is invalid.

The address of `appData`, `appData->owned` and `appData->resources` should be adjacent to each other, they happen to be pushed to the memory sequentially. If `appData` is located on `0x00000000` and `sizeof(AppData)` is 40, then `appData->owned` should start at `0x00000028`

But as it turns out the address of `appData->owned` and `appData->resources` points to invalid memory address and that address is particularly big.

For the avid readers you could already see why the bug happens given the information I give above.

```c
#define pushStruct(arena, type, ...) (type*)arena->pushSize_(sizeof(type), ## __VA_ARGS__)
#define pushArray(arena, count, type, ...) (type*)arena->pushSize_((count)*sizeof(type), ## __VA_ARGS__)
#define pushSize(arena, size, ...) arena->pushSize_(size, ## __VA_ARGS__) 

struct MemoryArena
{
    size_t size;
    size_t used;
    u8* base;

    size_t getAlignmentOffset(size_t alignment)
    {
        size_t alignmentOffset = 0;
        size_t pointer = (size_t)base + used;

        size_t alignmentMask = alignment - 1;
        if(pointer & alignmentMask)
        {
            alignmentOffset = alignment - (pointer & alignmentMask);
        }

        return alignmentOffset;
    }

    void* pushSize_(size_t inSize, size_t alignment/* = 8*/)
    {
        size_t alignmentOffset = getAlignmentOffset(alignment);
        inSize += alignmentOffset;

        void* result = base + used;
        used += inSize;
        
        return result;
    }
};
```

Everytime we do a `pushStruct` or `pushArrays`, first we need to check if the data that we’re pushing is aligned on the alignment boundary. If not, we’re
adding an offset to the end of the last pushed data. With this all write and reads to the memory guaranteed to be aligned with how the memory access it. It should not be happening because of alignment I guess, but why on the release build happens more frequently then?

...
### Making Wrong Assumptions
I tried changing the default alignment param for `pushSize_` from 8, to 4 and 16. Tested both on release and debug builds. Nothing changes, everything works until the first time I write the save data, then the next launch still crashes.

Then I tried not adding the `alignmentOffset` to the size, just to make sure.
Since I properly aligned all my data structs, it should still works correctly with my MacBook Pro 2015, that should be 64bit aligned right? But just to make sure since I use XCode and I don’t know what black secret magic compilations and optimizations that they’re used behind the scene, it could messed up my alignment or so I think. But still, no changes with the behavior of the bug.

At the very end I tried not to use the preallocated memory, but each data that is pushed is returned using malloced memory, and then the bug suddenly disappears.

Whoa if the bug disappears using `malloc`, then this defeats the purpose of using linear contiguos memory, preallocated on the beginning, since malloced memory happens to make the app works correctly. It should not be this way, I don’t want the idea of using `malloc` every time I need to allocate a memory, this is the same as using new. The only difference is that `new` allocate and init a memory while malloc only allocates it, grabs a sized memory and returned anything that is inside it, non-initialized.

So what is it that makes the `malloc` version works then? I thought for a bit. The difference between those two are one is linearly contiguous and the other address is distributed all over the place. Why is the idea of the address not adjacent to each other works then?

Then it hits me hard.

```c
struct AppData
{
    u16 id;
    u16 pad_1;
    u32 pad_2;

    u32 data1;
    u32 data2;
    u32 data3;
    u32 data4;

    OwnedResource* owned;
    Resource* resources;

};

{
    fread(appData, sizeof(AppData), 1, dataFile);

    // Crash happens here
    fread(appData->owned, sizeof(OwnedResource)*ResourceType_Count, 1, dataFile);
    fread(appData->resources, sizeof(Resource)*ResourceType_Count, 1, dataFile);
}
```

Reading appData with `sizeof(AppData)`, in this case it’s 40, also includes the data address inside `owned` and `resources`. So on the first run before we do a save data it will always works since the data is empty. But the next run after the save data is written, the address of those two pointers also gets written. Thus reading 40 bytes into `appData` also trash the contents of `appData->owned` and `appData->resources`. Previously we `pushStruct` arrays into the memory arena, but it is trashed after we read the appData. The alignment itself is never the problem, **the assumptions that it is happening because of it is the problem.**

### A Detour to My Backyard
From my experience, debugging is a process in which you try to investigate why something happens with your limited ideas that could come in mind.
Think of it as you are searching for a hidden lifetime savings buried on your backyard, the problem is, you’re the one that hid it and you don’t remember where you put it, as in a bug where the cause of it is usually yourself, your past-self is a fool and your future-self is forgetful.

You begin limiting your search with a particular rock on the corner.

Weird, you don’t remember you had a big rock in your lovely backyard.
You assume is it hidden beneath the rock, because when you check the surroundings area of the rock, the ground is soft, someone must have dug it and store it there recently, and someone is you.

You know think that you’re first assumptions is correct and thought you’re a genius.

You then begin digging and digging only to find your dead cat that you’ve buried weeks ago, you then cried deeply and become frustrated, repeat.

After a lot of time passed, you just realized that you’re savings is not buried on your backyard, it is beneath your pillow that you used at night when you’re weeping sadly about your dead cat. It’s been there all along, closer than you might think.

See the frustration that happens when finding a root cause of a bug happens because we makes wrong assumptions. So by making assumptions that the bug happens because of memory alignment, I happen to check on the wrong side of things. My assumptions also strengthened by the fact that it happens more on release build. Clearly release build usually do some optimization and more extra work by packing your data, thus a wrong access would cause access violations.

### Hold X to Give Respect To The Cat
There are two ways that I could think of to fix this bug, either by separating the pointers into another struct that holds only those two pointers, or reading and writing the data itself field by field. By separating the struct, the code turned into this

```c
struct AppData
{
    u16 id;
    u16 pad_1;
    u32 pad_2;

    u32 data1;
    u32 data2;
    u32 data3;
    u32 data4;
};

struct OtherData
{
    OwnedResource* owned;
    Resource* resources;
};

{
    AppData* appData = pushStruct(&arena, AppData);
    OtherData* otherData = pushStruct(&arena, OtherData);

    otherData->owned = pushArray(&arena, ResourceType_Count, OwnedResource);
    otherData->resources = pushArray(&arena, ResourceType_Count, Resource);
}
```
and the other way to fix this, is by changing the serialization code itself

```c
    fread(&appData->id, sizeof(u16), 1, dataFile);
    fread(&appData->pad_1, sizeof(u16), 1, dataFile);
    fread(&appData->pad_2, sizeof(u32), 1, dataFile);
    fread(&appData->data1, sizeof(u32), 1, dataFile);
    fread(&appData->data2, sizeof(u32), 1, dataFile);
    fread(&appData->data2, sizeof(u32), 1, dataFile);
    fread(&appData->data4, sizeof(u32), 1, dataFile);
    fread(appData->owned, sizeof(OwnedResource)*ResourceType_Count, 1, dataFile);
    fread(appData->resource, sizeof(Resource)*ResourceType_Count, 1, dataFile);
```

I happen to choose the later version to fix the problem. In my app, there are a lot places that `appData` would need to access `otherData`, and it could changes the way the data passes around the app if I happen to break those two together, so I choose the simplest way.

As a side note, I probably would change the serialization code into something
described here, they do some cool stuff with serializations.

### Putting The Search to a Rest
So as a general rule, by gaining a lot of experience, you’ve more likely sharpened your assumptions on a bug. This is why experienced developers tends to know quickly how and why a bug happens. They could easily locate which particular line of code that cause this and makes correct assumptions on where to check.

One of the books that I like and I would recommend given the chance is [The Art and Craft of Problem Solving](https://www.amazon.com/Art-Craft-Problem-Solving/dp/0471789011). That’ll give you insight if you have a block on how to solve a particular problem and force you either to think creatively or by thinking it from another point of view.

So for a last riddle, what if I ask you to complete this word but without using the letter ‘E’

**SEQUENC_**

Take your time to think of the answers, and anyway if you haven’t seen [Handmadehero](https://handmadehero.org/), go ahead check it out, you’ll learn a ton there.
