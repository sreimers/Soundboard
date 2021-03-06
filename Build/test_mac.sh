#!/bin/sh
echo "Update Git Submodules"
git submodule init > /dev/null
git submodule update > /dev/null

echo "Bootstrap Tools"
cd Submodules/JUCE/extras/Projucer/Builds/MacOSX/
xcodebuild -configuration Release > /dev/null
cd ../../../../../../

echo "Update Projects"
./Submodules/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer --resave Projects/Tests/Tests.jucer > /dev/null

echo "UnitTesting"
cd Projects/Tests/Builds/MacOSX/
xcodebuild -configuration Release > /dev/null
cd ../../../../
Projects/Tests/Builds/MacOSX/build/Release/Tests
