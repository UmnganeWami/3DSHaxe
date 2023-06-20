@echo off
if exist export (
    echo --------------------------
    echo Deleting the export folder.
    RMDIR "export" /S /Q
    mkdir "export"
)
echo --------------------------
echo Exporting haxe code.
haxe build.hxml
if errorlevel 1 (
    echo UNSUCCESSFUL HAXE COMPILE!
    echo check above for error!
    echo
    echo deleting the export folder!
    RMDIR "export" /S /Q
    echo --------------------------
) else (
    echo Haxe export successful!
    
    echo --------------------------
    echo Copying game assets
    xcopy /E "./assets" "./export" 
    echo --------------------------

    echo Compiling exported haxe code to 3ds rom!
    echo
    cd export
    make
    cd ..
    if errorlevel 1 (
        echo Make error! check above code for the error
        echo
        echo deleting the export folder!
        RMDIR "export" /S /Q
        echo --------------------------
    ) else (
        echo Done!
        echo --------------------------
    )
)
