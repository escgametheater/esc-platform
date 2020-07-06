Gotchas
=========

Compile Gloox as static i386 library
-----------
To compile Gloox 1.0.9, update the compiler flags to use the appropriate architecture, build types, and C++ library as follows:

$ ./configure CC="gcc -arch i386 -arch x86_64" CXX="g++ -arch i386 -arch x86_64" CPP="gcc -E" CXXCPP="g++ -E"

Re-register app with Launch Services
-----------
To update the Apple Launch Services db entry for an application that has had its CFBundleURLSchemes modified, use:

$ /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f ~/<application-path>


Disable Apple's Quarantine Feature
-----------
$ defaults write com.apple.LaunchServices LSQuarantine -bool NO
