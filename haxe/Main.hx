package;

import cxx.num.UInt32;
import Nintendo.C3DRenderTarget;
//import Nintendo.C2DRenderTarget;
import Nintendo.GfxSide_t;
import Nintendo.GfxScreen_t;
import cxx.Ptr; //.Pointer;


function main(){
    //trace("uwu");
    untyped __include__("3ds.h");
	//@:include("citro3d.h")
    untyped __include__("citro2d.h"); //
    untyped __include__("citro3d.h"); //

	Nintendo.gfxInitDefault();
	Nintendo.C3D_Init(Nintendo.C3D_DEFAULT_CMDBUF_SIZE);
	Nintendo.C2D_Init(Nintendo.C2D_DEFAULT_MAX_OBJECTS);
	Nintendo.C2D_Prepare();
	Nintendo.consoleInit(GfxScreen_t.GFX_BOTTOM, null);

	//var bottom = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_BOTTOM, GfxSide_t.GFX_LEFT);
	var bottomptr:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_BOTTOM, GfxSide_t.GFX_LEFT);

	var clearColor:UInt32 = Nintendo.C2D_Color32f(0,0,0,1);
	//C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    Nintendo.printf("Hello World\n");

    while(Nintendo.aptMainLoop()){
		Nintendo.C2D_TargetClear(bottomptr, clearColor);
        /*Nintendo.hidScanInput();

        Nintendo.gfxFlushBuffers();
        Nintendo.gfxSwapBuffers();

        Nintendo.gspWaitForVBlank();*/
    }
    Nintendo.gfxExit();
}

/*

#include <citro2d.h>
#include <3ds.h>


int main(int argc, char* argv[]) {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, NULL);

	C3D_RenderTarget* bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);


	u32 clear_color = C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f);

	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_SELECT)
			break;

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		C2D_TargetClear(bot, clear_color);
		C2D_SceneBegin(bot);

		C2D_DrawRectSolid(0, 0, 0, 40, 40, C2D_Color32f(1, 1, 1, 1));


		C2D_Flush();
		C3D_FrameEnd(0);
	}
	C2D_Fini();
	C3D_Fini();

	gfxExit();
	return 0;
}

*/