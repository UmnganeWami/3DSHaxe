package;




@:include("stdio.h")
@:include("stdlib.h")
@:include("3ds.h")
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

}