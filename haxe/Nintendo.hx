package;

import cxx.Ptr;
import cxx.num.SizeT;
import cxx.num.UInt32;
import cxx.num.UInt8;

//@:include("citro2d.h")

@:native("PrintConsole")
extern typedef PrintConsole = {};

enum GfxScreen_t {
	@:native("GFX_TOP")
	GFX_TOP;
	@:native("GFX_BOTTOM")
	GFX_BOTTOM;
}

enum GfxSide_t {
	@:native("GFX_LEFT")
	GFX_LEFT;
	@:native("GFX_RIGHT")
	GFX_RIGHT;
}

enum FrameBeginEnum {//idk what to call it lol
	@:native("C3D_FRAME_SYNCDRAW")
	C3D_FRAME_SYNCDRAW;
	@:native("C3D_FRAME_NONBLOCK")
	C3D_FRAME_NONBLOCK;
}
/*
enum
{
	C3D_FRAME_SYNCDRAW = BIT(0), // Perform C3D_FrameSync before checking the GPU status
	C3D_FRAME_NONBLOCK = BIT(1), // Return false instead of waiting if the GPU is busy
};
*/


//@:native("C2D_RenderTarget")
//class C2DRenderTarget {}

@:native("C3D_RenderTarget")
extern class C3DRenderTarget {}

//gfx3dSide_t

@:include("3ds.h")
@:include("citro2d.h")
@:include("citro3d.h")


class Nintendo{
	@:native("C3D_DEFAULT_CMDBUF_SIZE")
	public static var C3D_DEFAULT_CMDBUF_SIZE:SizeT;

	@:native("C2D_DEFAULT_MAX_OBJECTS")
	public static var C2D_DEFAULT_MAX_OBJECTS:SizeT;

    @:native("gfxInitDefault")
    public static function gfxInitDefault():Void{};

    @:native("printf")
    public static function printf(string:String):Void{};

	@:native("aptMainLoop")
	public static function aptMainLoop():Bool{return true;};

	@:native("gspWaitForVBlank")
	public static function gspWaitForVBlank():Void{};

	@:native("gfxSwapBuffers")
	public static function gfxSwapBuffers():Void{};

	@:native("gfxFlushBuffers")
	public static function gfxFlushBuffers():Void{};

	@:native("hidScanInput")
	public static function hidScanInput():Void{};

	@:native("gfxExit")
	public static function gfxExit():Void{};

	@:native("consoleInit")
	public static function consoleInit(screen:GfxScreen_t, printconsole:Null<Ptr<PrintConsole>>):Void{};

	@:native("C3D_Init")
	public static function C3D_Init(maxObjects:SizeT):Void{};

	@:native("C2D_Init")
	public static function C2D_Init(maxObjects:SizeT):Bool{return true;};

	@:native("C2D_Prepare")
	public static function C2D_Prepare():Void{};

	@:native("C3D_Prepare")
	public static function C3D_Prepare():Void{};

	@:native("C2D_CreateScreenTarget")
	public static function C2D_CreateScreenTarget(screen:GfxScreen_t, side:GfxSide_t):Ptr<C3DRenderTarget>{return null;};

	@:native("C2D_Color32f")
	public static function C2D_Color32f(r:Float, g:Float, b:Float, a:Float):UInt32{return 0;}; 

	@:native("C2D_TargetClear")
	public static function C2D_TargetClear(target:Ptr<C3DRenderTarget>, color:UInt32):Void{}; 

	@:native("C3D_FrameBegin")
	public static function C3D_FrameBegin(flags:FrameBeginEnum):Void{}; 

	@:native("C2D_SceneBegin")
	public static function C2D_SceneBegin(target:Ptr<C3DRenderTarget>):Void{}; 

	@:native("C2D_DrawRectSolid")
	public static function C2D_DrawRectSolid(x:Float, y:Float, z:Float, w:Float, h:Float, clr:UInt32):Void{}; 

	@:native("C2D_Flush")
	public static function C2D_Flush():Void{}; 

	@:native("C3D_FrameEnd")
	public static function C3D_FrameEnd(flags:UInt8):Void{}; 

}
//C2D_DrawRectSolid(0, 0, 0, 40, 40, C2D_Color32f(1, 1, 1, 1));

//consoleInit(GFX_BOTTOM, NULL);
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