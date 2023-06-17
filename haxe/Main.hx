package;


function main(){
    //trace("uwu");
    untyped __include__("3ds.h");
    untyped __include__("stdio.h");
    untyped __include__("stdlib.h");

    Nintendo.gfxInitDefault();
    untyped __cpp__("consoleInit(GFX_TOP, NULL)");
    
    Nintendo.printf("haiiii \n");
    Nintendo.printf("haiiii \n");
    Nintendo.printf("haiiii \n");
    while(Nintendo.aptMainLoop()){
        Nintendo.hidScanInput();

        Nintendo.gfxFlushBuffers();
        Nintendo.gfxSwapBuffers();

        Nintendo.gspWaitForVBlank();
    }
    Nintendo.gfxExit();
}