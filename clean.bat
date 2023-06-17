@echo off
RMDIR "export" /S /Q
mkdir export
xcopy "./compstuff" "./export"