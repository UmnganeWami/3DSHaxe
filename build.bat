@echo off
echo Exporting haxe project
haxe build.hxml
echo Haxe export done!
echo Compiling exported haxe code to 3ds rom!
if exist export/Makefile (
    echo Makefile found!
) else (
    echo Makefile not found, adding it in
    xcopy /E "./assets" "./export" 
)
cd export
make
cd .. 
echo Done!
