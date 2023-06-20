package;

import Nintendo.CWAV;
import cxx.Syntax;
import Nintendo.C2D_Sprite;
import Nintendo.C2D_SpriteSheet;
import cxx.num.UInt32;
import Nintendo.C3DRenderTarget;
import Nintendo.FrameBeginFlag;
import Nintendo.GfxSide_t;
import Nintendo.GfxScreen_t;
import Nintendo.Buttons;
import Nintendo.CwavEnvMode_t;
import cxx.Ptr;

var screenAreaHeight:Float = 240.0; //the height of both screens
var screenAreaWidth:Float = 400.0; //the width of both screens
function main(){
    untyped __include__("3ds.h");
    untyped __include__("citro2d.h"); 
    untyped __include__("citro3d.h");  
    untyped __include__("cwav.h");  

	Nintendo.romfsInit();
	Nintendo.gfxInitDefault();
	Nintendo.C3D_Init(Nintendo.C3D_DEFAULT_CMDBUF_SIZE);
	Nintendo.C2D_Init(Nintendo.C2D_DEFAULT_MAX_OBJECTS);
	Nintendo.C2D_Prepare();

	Nintendo.cwavUseEnvironment( CwavEnvMode_t.CWAV_ENV_DSP );
	Nintendo.ndspInit();

	var bottomScreen:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_BOTTOM, GfxSide_t.GFX_LEFT);
	var topScreen:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_TOP, GfxSide_t.GFX_LEFT);
	var clearColor:UInt32 = Nintendo.C2D_Color32f(0.0, 0.0, 0.0, 1.0);

	var sprsheet:C2D_SpriteSheet = Nintendo.C2D_SpriteSheetLoad("romfs:/gfx/test.t3x");
	var sprite:C2D_Sprite = Syntax.NoAssign;
	var spritePointer:Ptr<C2D_Sprite> = Syntax.toPointer(sprite);
	Nintendo.C2D_SpriteFromSheet(spritePointer,sprsheet,0);

	//var cwav:CWAV = Syntax.NoAssign;
	//var cwavPointer:Ptr<CWAV> = Syntax.toPointer(cwav);
	//Nintendo.cwavFileLoad(cwavPointer, "romfs:/beep_dsp_adpcm.bcwav", 3);

	var x:Float = 0;
    while(Nintendo.aptMainLoop()){
		Nintendo.gspWaitForVBlank();
		Nintendo.hidScanInput();
		//cwavPlay(cwav, 0, 1);
		//Nintendo.cwavPlay(cwavPointer, 0, -1);
		var btnd:UInt32 = Nintendo.hidKeysHeld();
		if (checkButton(btnd, Buttons.KEY_DLEFT)){
			x -= 1;
		}

		if (checkButton(btnd, Buttons.KEY_DRIGHT)){
			x += 1;
		}

		if (checkButton(btnd, Buttons.KEY_START)){
			break;
		}

		Nintendo.C2D_SpriteSetPos(spritePointer, x, 0);
		
		Nintendo.C3D_FrameBegin(FrameBeginFlag.C3D_FRAME_SYNCDRAW);
		Nintendo.C2D_TargetClear(topScreen, clearColor);
		Nintendo.C2D_SceneBegin(topScreen);

		//drawTop(); 
		Nintendo.C2D_DrawSprite(spritePointer);

		Nintendo.C3D_FrameBegin(FrameBeginFlag.C3D_FRAME_SYNCDRAW);
		Nintendo.C2D_TargetClear(bottomScreen, clearColor);
		Nintendo.C3D_FrameDrawOn(bottomScreen);

		//drawBottom();
		Nintendo.C2D_DrawSprite(spritePointer);

		Nintendo.C2D_Flush();
		Nintendo.C3D_FrameEnd(0);
    }
	Nintendo.C2D_SpriteSheetFree(sprsheet);
	//cwavFileFree(std::get<1>(*it));
	Nintendo.C2D_Fini();
	Nintendo.C3D_Fini();
    Nintendo.gfxExit();
	Nintendo.romfsExit();
}

function drawTop(){
	Nintendo.C2D_DrawRectSolid(0.0, 0.0, 0.0, screenAreaWidth, screenAreaHeight, Nintendo.C2D_Color32f(0.0, 1.0, 0.0, 1.0)); 
}

function drawBottom(){
	Nintendo.C2D_DrawRectSolid(0.0, 0.0, 0.0, screenAreaWidth, screenAreaHeight, Nintendo.C2D_Color32f(0.0, 1.0, 0.0, 1.0)); 
}

function checkButton(keysstuff:UInt32, key:UInt32):Bool{
	return untyped __cpp__("keysstuff & key");
}