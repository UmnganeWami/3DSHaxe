package;

import cpp.Pointer;

@:include("stdio.h")
@:include("stdlib.h")
@:include("3ds.h")
//@:include("citro2d.h")

@:native("PrintConsole")
typedef PrintConsole = {};

/*typedef enum {
  GFX_TOP = GSP_SCREEN_TOP, ///< Top screen
  GFX_BOTTOM = GSP_SCREEN_BOTTOM, ///< Bottom screen
 } gfxScreen_t;*/

enum GfxScreen_t {
	@:native("GFX_TOP")
	GFX_TOP;
	@:native("GFX_BOTTOM")
	GFX_BOTTOM;
}

class Nintendo{
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

	/*public static function consoleInit(screen:Int = 1){
		if(screen == 1){
			untyped __cpp__("consoleInit(GFX_TOP, NULL)");
		}else{
			untyped __cpp__("consoleInit(GFX_BOTTOM, NULL)");
		}
	}*/
	@:native("consoleInit")
	public static function consoleInit(screen:GfxScreen_t, printconsole:Null<Pointer<PrintConsole>>):Void{};

}

//consoleInit(GFX_BOTTOM, NULL);