+++
title = 'Symbolicate ios Submission Crash Report'
date = 2018-01-20T10:00:00+07:00
toc = false 
+++

Often times we get crash from ios submission report that goes like this

```toml
Thread 0 name:  Dispatch queue: com.apple.main-thread
Thread 0 Crashed:
0   cong2                         	0x0000000101e78060 0x100bbc000 + 19644512
1   cong2                         	0x0000000100bcfb84 0x100bbc000 + 80772
2   cong2                         	0x0000000100bcff28 0x100bbc000 + 81704
3   CoreFoundation                	0x00000001854e113c __CFNOTIFICATIONCENTER_IS_CALLING_OUT_TO_AN_OBSERVER__ + 20 (CFNotificationCenter.c:661)
4   CoreFoundation                	0x00000001854e06dc _CFXRegistrationPost + 420 (CFNotificationCenter.c:163)
5   CoreFoundation                	0x00000001854e0440 ___CFXNotificationPost_block_invoke + 60 (CFNotificationCenter.c:1060)
6   CoreFoundation                	0x000000018555de24 -[_CFXNotificationRegistrar find:object:observer:enumerator:] + 1408 (CFXNotificationRegistrar.m:163)
7   CoreFoundation                	0x0000000185416d60 _CFXNotificationPost + 380 (CFNotificationCenter.c:1057)
```

Currently we don’t know what is the cause of the crash, since the top three address shown above is not symbolicated (only showing address of the function).

Reading only the address of the function clearly is not helping, so we need to symbolicate the address into our corresponding code, but how do we do that?

---

First we need the symbol used from the build submitted, say you upload an iphone build with version 1.0.0, we need to locate the exact version 1.0.0 of the archive file. Using a different version won’t work because the crash report address only correspond to the correct binary and symbol pair.

Another way to obtain the symbol is to use the download dSYM button on XCode Organizer window while selecting the target archive.

After you locate the archive file, next we need to locate the symbol dSYM file.
So on finder, right click on archive file, Show Package Contents then grab the dSYM file located on the dSYMS folder, usually the symbol would be named yourApp.app.dSYM

In this case we’re using cong archive file, so copy cong.app.dSYM get that file from the archive, we need to poke it a little further.

Open your favourite terminal, go to the locating symbol folder, then run

```properties
atos -arch arm64 -o cong2.app.dSYM/Contents/Resources/DWARF/cong2 -l 0x100bbc000 0x0000000101e78060
```

those last two address are the topmost address shown from the crash report example, running those would show this result


```properties
ernesernesto@erness-MBP > ~/Desktop/symbolicate > atos -arch arm64 -o cong2.app.dSYM/Contents/Resources/DWARF/cong2 -l 0x100bbc000 0x0000000101e78060
UnityUpdateDisplayList (in cong2) (LibEntryPoint.mm:578)
```

Running the rest of the two unsymbolicated address show this result

```properties
ernesernesto@erness-MBP > ~/Desktop/symbolicate > atos -arch arm64 -o cong2.app.dSYM/Contents/Resources/DWARF/cong2 -l 0x100bbc000 0x0000000100bcfb84 
DisplayManager updateDisplayListInUnity] (in cong2) (DisplayManager.mm:330)ernesernesto@erness-MBP > ~/Desktop/symbolicate > atos -arch arm64 -o cong2.app.dSYM/Contents/Resources/DWARF/cong2 -l 0x100bbc000 0x0000000100bcff28 
DisplayManager screenDidConnect:] (in cong2) (DisplayManager.mm:369)
```

From here, you could fill in blanks for the missing stack trace crash


```toml
0   cong2                         	0x0000000101e78060  UnityUpdateDisplayList (in cong2) (LibEntryPoint.mm:578)
1   cong2                         	0x0000000100bcfb84  DisplayManager updateDisplayListInUnity] (in cong2) (DisplayManager.mm:330)
2   cong2                         	0x0000000100bcff28  DisplayManager screenDidConnect:] (in cong2) (DisplayManager.mm:369)
3   CoreFoundation                	0x00000001854e113c __CFNOTIFICATIONCENTER_IS_CALLING_OUT_TO_AN_OBSERVER__ + 20 (CFNotificationCenter.c:661)
4   CoreFoundation                	0x00000001854e06dc _CFXRegistrationPost + 420 (CFNotificationCenter.c:163)
5   CoreFoundation                	0x00000001854e0440 ___CFXNotificationPost_block_invoke + 60 (CFNotificationCenter.c:1060)
6   CoreFoundation                	0x000000018555de24 -[_CFXNotificationRegistrar find:object:observer:enumerator:] + 1408 (CFXNotificationRegistrar.m:163)
7   CoreFoundation                	0x0000000185416d60 _CFXNotificationPost + 380 (CFNotificationCenter.c:1057)```
```

Now go ahead, fix those crash happily (or not).

For complete reference of this symbolicate mumbo-jumbo 
see this [link](https://developer.apple.com/library/content/technotes/tn2151/_index.html#//apple_ref/doc/uid/DTS40008184-CH1-SYMBOLICATEWITHXCODE)
