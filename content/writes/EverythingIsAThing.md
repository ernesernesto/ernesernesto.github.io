+++
title = "Everything is a Thing: How I Built Match Morphosis in Pure C"
date = 2026-04-28T11:56:25+07:00
toc = false
+++

I've been building [Match Morphosis](https://store.steampowered.com/app/4133530/Match_Morphosis_Demo/) solo in plain C. No engine, no framework, just C and a handful of carefully chosen libraries. This post is about two decisions that ended up shaping everything else in the codebase: the entity model and the memory model.

Both of these came from the same instinct — keep things explicit, keep things visible, don't let the runtime surprise you. If you've watched Anton Mikhailov talk on the [Wookash Podcast](https://www.youtube.com/@WookashPodcast), you'll recognize the general direction. Nothing entirely new, they just hashed it out well in those episodes. Worth watching.

---

## Everything is a Thing

The first decision was how to represent game objects. In most codebases you end up with a zoo of types — enemies in one system, UI buttons in another, particles somewhere else, each with their own allocation and lifecycle logic. I wanted one model that covered everything.

Every object you see in the game — tiles, armaments, enemies, buttons, particles, progress bars — is an instance of one type:

    struct Thing
    {
        union
        {
            ObjectHandle  o;
            Piece         piece;
            Armament      armament;
            Player        player;
            Enemy         enemy;
            Particle      particle;
            Button        button;
            ProgressBar   progressBar;
            // ...
        };
    };

One type. One flat pool. The union means every `Thing` slot is the same size, so the pool is just a plain array — no variable-length allocation, no scatter.

Every operation goes through a generational handle:

    typedef struct ThingHandle
    {
        i32 id;
        i32 generation;
    } ThingHandle;

The `id` is a slot index into the pool. The `generation` is a counter that bumps every time that slot is reused. Together they let you hold a reference to something without it going dangling on you when that thing dies.

The backing container keeps everything together:

    struct {
        Thing pool[THING_COUNT];
        b32   used[THING_COUNT];
        i32   generations[THING_COUNT];
        i32   firstFree;
        i32   nextFree[THING_COUNT];
        i32   freeCount;
    } thingContainer;

Allocation pulls a slot from the free list and bumps that slot's generation:

    static ThingHandle thingMake(void)
    {
        ThingHandle result = {0};
        i32 slot = game->thingContainer.firstFree;
        if (game->thingContainer.firstFree)
        {
            game->thingContainer.used[slot] = true;
            game->thingContainer.generations[slot] += 1;
            game->thingContainer.freeCount--;
            result.id         = slot;
            result.generation = game->thingContainer.generations[slot];
            swMemset(&game->thingContainer.pool[slot], 0, sizeof(Thing));
            game->thingContainer.firstFree = game->thingContainer.nextFree[slot];
        }
        else
        {
            LOG("Thing count larger than config pool");
            result = game->zeroThing;
        }
        return result;
    }

Each concrete type has its own make function — `thingMakePiece()`, `thingMakeEnemy()` and so on — which call `thingMake()` underneath and then initialize their relevant union member. The caller gets a handle back, never a raw pointer.

### Handle resolution and ZeroThing

When you dereference a handle, the generation stored in the handle is compared against `thingContainer.generations[id]`. If they match, you get the live object. If they don't match — meaning that slot was freed and reused for something else since you last touched it — you get `ZeroThing` back instead.

`ZeroThing` is slot 0 of the pool, treated as a permanent sentinel. It's always zeroed, it returns safe defaults for everything, and it never crashes. You can hold a stale handle to a dead enemy across as many frames as you want. Worst case is you're talking to a zero struct, not reading garbage memory or segfaulting.

This eliminates a whole category of defensive null checks from gameplay code. You don't need to ask "is this thing still alive?" before every access. You just use it, and if it's gone, the answer comes back as nothing interesting.

This pattern is unremarkable to write in C. No base classes, no vtable, no factory pattern. It's just a struct and an array.

### Future plan: typed handles

Right now everything uses `ThingHandle`. The next step is distinct handle types per kind — `PieceHandle`, `EnemyHandle`, `ButtonHandle` — so that passing the wrong one to a function is caught at compile time rather than turning into a runtime bug. bgfx does exactly this with its handle types. In C you get most of the way there for free: `typedef struct { i32 id; i32 generation; } PieceHandle;` is a distinct type that the compiler won't silently coerce, zero runtime cost.

---

## One VirtualAlloc. That's it.

At startup the game calls `VirtualAlloc` once to reserve the full working set (~128MB — audio assets are the dominant cost, I haven't optimized that part yet). After that, there are no more allocation calls. Ever. `malloc` is never called.

A buddy allocator subdivides that block. The buddy system gives fast, predictable coalescing without fragmentation problems, and because all allocations come from one contiguous reservation, every pointer in the system is stable — just an offset from a known base.

The buddy allocator is also passed directly as the custom allocator into **bgfx** and **miniaudio**, so those libraries also draw from the same reservation rather than going to the heap behind your back. `thingContainer` lives in there too. Everything — rendering, audio, gameplay state — is in one flat address space with full visibility.

### Hot reload

Because all game state lives at stable offsets inside one contiguous block, hot reloading gameplay code is straightforward: unload the DLL, load the new one, hand it the same base pointer. The new code finds all the state exactly where it left it. No serialization step. No "save before reload". The memory layout is the save state.

Compile times sit around 2 seconds, so the loop is: change code, hit reload, see the result immediately inside the running game. For comparison — how long does a Play build take in Unity? This is what makes iteration actually enjoyable rather than something you dread.

One caveat worth mentioning: bgfx is compiled into the game DLL, so after a hot reload you need to re-set the bgfx context. This sounds annoying but it's a small addition to the bgfx source and it works cleanly.

### Numbers

- ~120MB total footprint (audio not yet optimized)
- 250ms cold launch to playable
- No loading screen

---

## Why plain C

The question I get most is why C and not C++. The honest answer is that C forces you to be explicit about everything. There are no hidden allocations, no constructors firing at surprising times, no implicit copies. When something happens, you wrote the code that made it happen. That visibility is worth a lot when you're trying to reason about performance and memory.

I remember something along these lines being written up by OurMachinery in the past — the "one big allocation" approach to game memory — but I can't track down the exact post. The ideas aren't new. Anton Mikhailov covers similar ground well in his recent Wookash Podcast appearances if you want to hear it argued from first principles. 

Aniway, doing it in C actually was way more enjoyable and fun rather than using Modern C++ or C#, one thing I missed with using C is probably destructors or a way to do a clean defer. Sometimes when I used stb_ds dynamic arrays I forget to free it, so a defer / destructor would really help in this part, but nothing that a quick grep would quickly solve if I'm missing some cleanup.

---

The demo is live on Steam, feel free to try it and leave feedback, I read every comment.
