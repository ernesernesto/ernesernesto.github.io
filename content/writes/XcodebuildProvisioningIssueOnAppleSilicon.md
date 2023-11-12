+++
title = 'xcodebuild Provisioning Issue On Apple Silicon'
date = 2023-11-12T15:29:18+07:00
toc = false
draft = false
+++

The company I worked on recently bought a new Mac Mini M2 to replace the old Mbp that is used as a jenkins driver to build all of our games on ios. 
At that time around 3-years ago, we don't have a dedicated mac mini that is used for building ios / android game and pandemic has just started, so we figured out we need to have a dedicated machine in order to unblock our programmers that need to have a working build on a device. We then opted to using our old mbp to use, one for android and one for ios.

The setup was quite simple, it's just a jenkins build that run our python build script that runs and package our build data, build the unity project into an xcode / android studio project, generate .ipa or apk and then finally upload it into app center. 

The jenkins work are triggered via webhooks, which is triggered when a particular branch of the git project is pushed to origin. 
Every game project has it's own jenkins workspace and this is quite a simple setup that could be configured on an 30mins work.

Fast forward 3 years later, we need to replace the old jenkins machine for ios with a brand new Mac Mini M2. The migration to the new machine is quite easy with the help of Mac Migration Assistant, took about half a day migration and I was quite surprised that most of the old tooling on the old mbp was working. Knowing that the old mbp is Intel-based chip and the Mac Mini is the new and shiny Apple-silicon based I was expecting a lot of tools that is installed is not working, but the issue was quite few and could be fixed with a couple of brew install and reinstall.

Apparently when we try to build .ipa again with the new M2, it fails when we try to archive the xcode project with this error
`"Provisioning profile XX doesn't include the currently selected device 'Mac' (identifier xx)"`

We are running the archiving and exporting to ipa via xcodebuild tools like this.

```
"xcodebuild \
    -quiet \
    -workspace Unity-iPhone.xcworkspace \
    -scheme Unity-iPhone archive \
    -archivePath ./ \
    -allowProvisioningUpdates"
```

Weirdly enough when we try to archive it by opening it via XCode, the archiving succeeds. Surely there's something new that is introduced when we use the new M2 machine that makes xcodebuild fails. After reading through some issues on the apple developer forum, the most recommended answer is to add the new Mac UDID into the provisioning that we use. 

That is surely non-optimal since firstly we don't want to build a mac desktop application, and secondly the archive actually works when we do it via XCode, we just don't want to open XCode manually when we want to create a build, that would defeat the purpose of making a CI/CD autobuild.

Upon further inspection and wasted hours reading through xcodebuild man pages. The solution is to just add `-destination ‘generic/platform=iOS’` to the xcodebuild parameters, thanks to this info [here](https://keith.github.io/xcode-man-pages/xcodebuild.1.html)


The complete working command for xcodebuild on mac apple silicon is this
```
"xcodebuild \
    -quiet \
    -workspace Unity-iPhone.xcworkspace \
    -scheme Unity-iPhone archive \
    -archivePath ./ \
    -allowProvisioningUpdates \
    -destination 'generic/platform=iOS'"
```

