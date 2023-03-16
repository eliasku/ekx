- CMake v3.19+
- NodeJS v18+
- NPM v9+
- Ninja Build
- fastlane
- ts-node / swc / esbuild (as dependency)

## Platforms

### Android
- Android Studio (required!): this includes required gradle, java, android SDK, android NDK

### iOS
- XCode (required!): this includes required swift
- cocoapods (required!)
- [XCodeGen](https://github.com/yonaskolb/XcodeGen) generates XCode project

### Web
- Emscripten SDK (required!)
- Firebase CLI (optional for deploy to Web hosting)

## Engine

### Libraries used in core runtime
- stb
- sokol
- drlib
- imgui
- oboe (for Android audio)
- quickjs (optional)
- pocketmod (optional)
- box2d (optional)

### Libraries used in content export utility
- fast_obj
- Freetype2
- Cairo
- pugixml
- miniz



