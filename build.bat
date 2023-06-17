@echo off
echo Exporting haxe project
haxe build.hxml
echo Haxe export done!
echo Compiling exported haxe code to 3ds rom!
cd export
make
cd .. 
echo Done!
