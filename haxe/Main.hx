package;

import cxx.num.Int32;
import Nintendo.C2D_Sprite;
import Nintendo.C2D_SpriteSheet;
import cxx.num.UInt32;
import Nintendo.C3DRenderTarget;
import Nintendo.FrameBeginEnum;
import Nintendo.GfxSide_t;
import Nintendo.GfxScreen_t;
import Nintendo.Buttons;
import Type;
import cxx.Ptr;

var height:Float = 240.0; //the height of both screens
var width:Float = 400.0; //the width of both screens
function main(){
    untyped __include__("3ds.h");
    untyped __include__("citro2d.h"); 
    untyped __include__("citro3d.h"); 
	Nintendo.romfsInit();
	Nintendo.gfxInitDefault();
	Nintendo.C3D_Init(Nintendo.C3D_DEFAULT_CMDBUF_SIZE);
	Nintendo.C2D_Init(Nintendo.C2D_DEFAULT_MAX_OBJECTS);
	Nintendo.C2D_Prepare();

	var bottomptr:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_BOTTOM, GfxSide_t.GFX_LEFT);
	var topptr:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_TOP, GfxSide_t.GFX_LEFT);
	var clearColor:UInt32 = Nintendo.C2D_Color32f(0.0, 0.0, 1.0, 1.0);

	var sprsheet:C2D_SpriteSheet = Nintendo.C2D_SpriteSheetLoad("romfs:/gfx/test.t3x");
	var spr:Null<C2D_Sprite> = null;
	untyped __cpp__("C2D_Sprite spr");
	//we want to not need the untyped __cpp__ thingy, so we'll have to fix that up later :3
	var sprPtr:Ptr<C2D_Sprite> = untyped __cpp__("&spr");
	Nintendo.C2D_SpriteFromSheet(sprPtr,sprsheet,0);


	var x:Float = 0;
    while(Nintendo.aptMainLoop()){
		Nintendo.gspWaitForVBlank();
		Nintendo.hidScanInput();

		var btnd:UInt32 = Nintendo.hidKeysHeld();
		if (checkButton(btnd, Buttons.KEY_DLEFT)){
			x -= 1;
		}

		if (checkButton(btnd, Buttons.KEY_DRIGHT)){
			x += 1;
		}

		Nintendo.C2D_SpriteSetPos(sprPtr, x, 0);
		
		Nintendo.C3D_FrameBegin(FrameBeginEnum.C3D_FRAME_SYNCDRAW);
		Nintendo.C2D_TargetClear(topptr, clearColor);
		Nintendo.C2D_SceneBegin(topptr);
		//drawTop(); 
		Nintendo.C2D_DrawSprite(sprPtr);

		Nintendo.C3D_FrameBegin(FrameBeginEnum.C3D_FRAME_SYNCDRAW);
		Nintendo.C2D_TargetClear(bottomptr, clearColor);
		Nintendo.C3D_FrameDrawOn(bottomptr);
		//drawBottom();
		Nintendo.C2D_DrawSprite(sprPtr);

		Nintendo.C2D_Flush();
		Nintendo.C3D_FrameEnd(0);
    }
    Nintendo.gfxExit();
}

function drawTop(){
	Nintendo.C2D_DrawRectSolid(0.0, 0.0, 0.0, width, height, Nintendo.C2D_Color32f(0.0, 1.0, 0.0, 1.0)); 
}

function drawBottom(){
	Nintendo.C2D_DrawRectSolid(0.0, 0.0, 0.0, width, height, Nintendo.C2D_Color32f(0.0, 1.0, 0.0, 1.0)); 
}

function checkButton(keysstuff:UInt32, key:UInt32):Bool{
	return untyped __cpp__("keysstuff & key");
}