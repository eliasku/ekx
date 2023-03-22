# Install on macOS

## Install XCode
Be sure you have activated "XCode" in "Command line tools" preference
Run `clang --version`, you should see XCode toolchain is used: `InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin`

## Install NodeJS and NPM

NodeJS 18+ and NPM 9+ is required

## Install build tools

Install [Homebrew](https://brew.sh/)

With brew you can install: CMake, Ninja build and CocoaPods:
```shell
brew install cmake ninja cocoapods`
```

Check versions:
```shell
$ ninja --version
1.11.1

$ cmake --version
cmake version 3.26.0
...

$ pod --version
...
1.11.3
```

NOTE: builtin cocoapods could be outdated, so you will need remove old bin:
```bash
$ brew link --overwrite cocoapods
```
