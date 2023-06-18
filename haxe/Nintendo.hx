package;

import cxx.Ptr;
import cxx.num.SizeT;
import cxx.num.UInt32;
import cxx.num.UInt8;

//@:include("citro2d.h")

@:native("PrintConsole")
extern class PrintConsole {}

@:native("Tex3DS_Texture")
extern class Tex3DS_Texture {}

@:native("C3D_Tex")
extern class C3D_Tex {}

@:native("C3D_TexCube")
extern class C3D_TexCube {}

@:native("C2D_Image")
extern class C2D_Image {}

@:native("C2D_SpriteSheet")
extern class C2D_SpriteSheet {}

@:native("C2D_Sprite")
extern class C2D_Sprite {}

//Tex3DS_Texture

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

enum GPU_COLORBUF {
	@:native("GPU_RB_RGBA8")
	GPU_RB_RGBA8;
	@:native("GPU_RB_RGB8")
	GPU_RB_RGB8;
	@:native("GPU_RB_RGBA5551")
	GPU_RB_RGBA5551;
	@:native("GPU_RB_RGB565")
	GPU_RB_RGB565;
	@:native("GPU_RB_RGBA4")
	GPU_RB_RGBA4;
}

enum GPU_DEPTHBUF {
	@:native("GPU_RB_DEPTH16")
	GPU_RB_DEPTH16;
	@:native("GPU_RB_DEPTH24")
	GPU_RB_DEPTH24;
	@:native("GPU_RB_DEPTH24_STENCIL8")
	GPU_RB_DEPTH24_STENCIL8;
}

enum C3D_ClearBits {
	@:native("C3D_CLEAR_COLOR")
	C3D_CLEAR_COLOR;
	@:native("C3D_CLEAR_DEPTH")
	C3D_CLEAR_DEPTH;
	@:native("C3D_CLEAR_ALL")
	C3D_CLEAR_ALL;
}

/*
typedef struct
{
	union
	{
		void* data;
		C3D_TexCube* cube;
	};

	GPU_TEXCOLOR fmt : 4;
	size_t size : 28;

	union
	{
		u32 dim;
		struct
		{
			u16 height;
			u16 width;
		};
	};

	u32 param;
	u32 border;
	union
	{
		u32 lodParam;
		struct
		{
			u16 lodBias;
			u8 maxLevel;
			u8 minLevel;
		};
	};
} C3D_Tex;
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

	@:native("C3D_RenderTargetCreate")
	public static function C3D_RenderTargetCreate(width:Int, height:Int, colorbuf:GPU_COLORBUF, depthbuf:GPU_DEPTHBUF):Ptr<C3DRenderTarget>{return null;};

	@:native("C2D_Color32f")
	public static function C2D_Color32f(r:Float, g:Float, b:Float, a:Float):UInt32{return 0;}; 

	@:native("C2D_TargetClear")
	public static function C2D_TargetClear(target:Ptr<C3DRenderTarget>, color:UInt32):Void{}; 

	@:native("C3D_RenderTargetClear")
	public static function C3D_RenderTargetClear(target:Ptr<C3DRenderTarget>, clear:C3D_ClearBits, color:UInt32, depth:UInt32):Void{}; 

	@:native("C3D_RenderTargetSetOutput")
	public static function C3D_RenderTargetSetOutput(target:Ptr<C3DRenderTarget>, screen:GfxScreen_t, side:GfxSide_t, transferFlags:UInt32):Void{}; 

	//@:native("sceneRender")
	//public static function sceneRender(target:Ptr<C3DRenderTarget>):Void{};

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

	@:native("C3D_FrameDrawOn")
	public static function C3D_FrameDrawOn(screen:Ptr<C3DRenderTarget>):Void{}; 

	@:native("romfsInit")
	public static function romfsInit():Void{}; 

	@:native("C2D_SpriteSheetLoad")
	public static function C2D_SpriteSheetLoad(path:String):C2D_SpriteSheet{return null;}; 
 
	@:native("Tex3DS_TextureImport")
	public static function Tex3DS_TextureImport(input:Ptr<Void>, insize:SizeT, tex:Ptr<C3D_Tex>, texcube:Null<Ptr<C3D_Tex>>, vram:Bool):Tex3DS_Texture{return null;}; 

	@:native("C2D_SpriteFromSheet")
	public static function C2D_SpriteFromSheet(sprites:Ptr<C2D_Sprite>, sheet:C2D_SpriteSheet, index:SizeT):Void{};
	//C2D_SpriteFromImage C2D_Sprite *sprite, C2D_Image image
	@:native("C2D_SpriteFromImage")
	public static function C2D_SpriteFromImage(sprite:Ptr<C2D_Sprite>, image:C2D_Image):Void{};


	@:native("C2D_DrawSprite")
	public static function C2D_DrawSprite(sprites:Ptr<C2D_Sprite>):Void{};  

	@:native("C2D_SpriteSheetCount")
	public static function C2D_SpriteSheetCount(sheet:C2D_SpriteSheet):SizeT{return 0;};

	@:native("C2D_SpriteSheetGetImage")
	public static function C2D_SpriteSheetGetImage(sheet:Null<C2D_SpriteSheet>, index:SizeT):C2D_Image{return null;};

	@:native("C2D_SpriteSetPos")
	public static function C2D_SpriteSetPos(sheet:Ptr<C2D_Sprite>, x:Float, y:Float):Void{};

}
//C2D_SpriteSetPos(&sprites, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

//consoleInit(GFX_BOTTOM, NULL);
/*
int main()
{
	// Initialize graphics
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	// Initialize the render target
	C3D_RenderTarget* top = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(top, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTarget* bot = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(bot, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	// Initialize the scene
	sceneInit();

	// Main loop
	float count = 0.0f;
	while (aptMainLoop())
	{
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C3D_RenderTargetClear(top, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			C3D_FrameDrawOn(top);
			sceneRender(count, true);
			C3D_RenderTargetClear(bot, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			C3D_FrameDrawOn(bot);
			sceneRender(count, false);
		C3D_FrameEnd(0);
		count += 1/128.0f;
	}

	// Deinitialize the scene
	sceneExit();

	// Deinitialize graphics
	C3D_Fini();
	gfxExit();
	return 0;
}
*/