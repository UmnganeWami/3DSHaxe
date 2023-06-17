@echo off
echo Deleting the export folder
RMDIR "export" /S /Q
./build
