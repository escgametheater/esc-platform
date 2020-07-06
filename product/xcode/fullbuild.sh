#!/bin/sh

xcodebuild -project EscPlatform/EscPlatform.xcodeproj -target EscPlatform-iOS -configuration Debug
xcodebuild -project EscPlatform/EscPlatform.xcodeproj -target EscPlatform-iOS -configuration Release
xcodebuild -project EscPlatform/EscPlatform.xcodeproj -target EscPlatform-Bundle -configuration Debug
xcodebuild -project EscPlatform/EscPlatform.xcodeproj -target EscPlatform-Bundle -configuration Release

# build tests...
# xcodebuild -project EscPlatform/EscPlatform.xcodeproj -target SystemTest -configuration Release

# /Applications/Unity/Unity.app/Contents/Frameworks/Mono/bin/
# gmcs -r:/Applications/Unity/Unity.app/Contents/Frameworks/Managed/UnityEngine.dll -target:library ./Esc/*.cs ./Osc/*.cs -out:Esc.dll