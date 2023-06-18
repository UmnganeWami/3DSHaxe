package;

import Nintendo.C2D_Sprite;
import Nintendo.C2D_SpriteSheet;
import cxx.num.UInt32;
import Nintendo.C3DRenderTarget;
import Nintendo.FrameBeginEnum;
import Nintendo.GfxSide_t;
import Nintendo.GfxScreen_t;
import Nintendo.Tex3DS_Texture;
import Nintendo.C2D_Image;
import Nintendo.C3D_Tex;
import cxx.Ptr;

typedef Sprite = {
	var spr:C2D_Sprite;
	var dx:Float;
	var dy:Float;
}

/**
 * typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;
 */
/*var topHeight:Float = 240.0;
var topWidth:Float = 800.0;
var bottomHeight:Float = 240.0;
var bottomWidth:Float = 400.0;*/
var height:Float = 240.0; //the height of both screens
var width:Float = 400.0; //the width of both screens
//var sprites:Map<Int, String>;
function main(){
    untyped __include__("3ds.h");
    untyped __include__("citro2d.h"); 
    untyped __include__("citro3d.h"); 
	Nintendo.romfsInit();
	Nintendo.gfxInitDefault();
	Nintendo.C3D_Init(Nintendo.C3D_DEFAULT_CMDBUF_SIZE);
	Nintendo.C2D_Init(Nintendo.C2D_DEFAULT_MAX_OBJECTS);
	Nintendo.C2D_Prepare();
	//Nintendo.consoleInit(GfxScreen_t.GFX_BOTTOM, null);

	var bottomptr:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_BOTTOM, GfxSide_t.GFX_LEFT);
	var topptr:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_TOP, GfxSide_t.GFX_LEFT);
	var clearColor:UInt32 = Nintendo.C2D_Color32f(0.0, 0.0, 1.0, 1.0);

	//var sprsheet:C2D_SpriteSheet = Nintendo.C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x"); //romfs:/gfx/sprites.t3x
	//untyped __cpp__("if (!sprsheet) svcBreak(USERBREAK_PANIC)");
	//var sprPtr:Ptr<C2D_Sprite> = null;
	//THIS CRASHES!
	//Nintendo.C2D_SpriteFromSheet(sprPtr,sprsheet,1);
	//var img:C2D_Image = Nintendo.C2D_SpriteSheetGetImage(sprsheet, 1);
	
	//THIS ALSO CRASHES!
	//Nintendo.C2D_SpriteFromImage(sprPtr, img);

    while(Nintendo.aptMainLoop()){
		Nintendo.gspWaitForVBlank();
		Nintendo.hidScanInput();
		
		Nintendo.C3D_FrameBegin(FrameBeginEnum.C3D_FRAME_SYNCDRAW);
		Nintendo.C2D_TargetClear(topptr, clearColor);
		Nintendo.C2D_SceneBegin(topptr);
		drawTop(); 
		//Nintendo.C2D_DrawSprite(sprPtr);
		//Nintendo.C2D_Flush();
		//Nintendo.C3D_FrameEnd(0);

		Nintendo.C3D_FrameBegin(FrameBeginEnum.C3D_FRAME_SYNCDRAW);
		Nintendo.C2D_TargetClear(bottomptr, clearColor);
		Nintendo.C3D_FrameDrawOn(bottomptr);
		drawBottom();

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

/*@:include("test_t3x.h")
class Sprites{
	@:native("test_t3x")
	@:noDiscard
	public static var test_t3x:Dynamic;
	@:native("test_t3x_size")
	public static var test_t3x_size:Dynamic;
}*/