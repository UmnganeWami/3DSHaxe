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
	Nintendo.consoleInit(GfxScreen_t.GFX_BOTTOM, null);

	Nintendo.cwavUseEnvironment( CwavEnvMode_t.CWAV_ENV_DSP );
	Nintendo.ndspInit(); //ncsndInit(true);

	var bottomScreen:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_BOTTOM, GfxSide_t.GFX_LEFT);
	var topScreen:Ptr<C3DRenderTarget> = Nintendo.C2D_CreateScreenTarget(GfxScreen_t.GFX_TOP, GfxSide_t.GFX_LEFT);
	var clearColor:UInt32 = Nintendo.C2D_Color32f(0.0, 0.0, 0.0, 1.0);

	var sprsheet:C2D_SpriteSheet = Nintendo.C2D_SpriteSheetLoad("romfs:/gfx/test.t3x");
	var sprite:C2D_Sprite = Syntax.NoAssign;
	var spritePointer:Ptr<C2D_Sprite> = Syntax.toPointer(sprite);
	Nintendo.C2D_SpriteFromSheet(spritePointer,sprsheet,0);

	var cwav:CWAV = Syntax.NoAssign;
	var cwavPointer:Ptr<CWAV> = Syntax.toPointer(cwav);
	Nintendo.cwavFileLoad(cwavPointer, "romfs:/beep_dsp_adpcm.bcwav", 1);
	Nintendo.printf("meow?");
	untyped __cpp__("if (cwavPointer->loadStatus == CWAV_SUCCESS)
	{
		printf(\"FUCKKKK\");
	}");

	var x:Float = 0;
    while(Nintendo.aptMainLoop()){
		Nintendo.gspWaitForVBlank();
		Nintendo.hidScanInput();
		//Nintendo.cwavPlay(cwavPointer, 0, 1);
		//cwavPlay(cwav, 0, 1);
		var btnd:UInt32 = Nintendo.hidKeysHeld();
		var btnp:UInt32 = Nintendo.hidKeysDown();
		if (checkButton(btnp, Buttons.KEY_DLEFT)){
			Nintendo.cwavPlay(cwavPointer, 0, -1);
			x -= 1;
		}

		if (checkButton(btnp, Buttons.KEY_DRIGHT)){
			Nintendo.consoleClear();
			//print_u32_binary(Nintendo.cwavGetEnvironmentPlayingChannels());
			x += 1;
		}

		if (checkButton(btnp, Buttons.KEY_START)){
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
		//Nintendo.C2D_DrawSprite(spritePointer);

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

//IM TOO LAZY TO REWRITE THIS IN HAXE!!
/*function print_u32_binary(val:UInt32){
	untyped __cpp__("
	char* bit_str[] = 
{
    \"0000\", \"0001\", \"0010\", \"0011\",
    \"0100\", \"0101\", \"0110\", \"0111\",
    \"1000\", \"1001\", \"1010\", \"1011\",
    \"1100\", \"1101\", \"1110\", \"1111\"
};
	for (u32 i = 0; i < 4; i++)
	{
		u32 toprint = val >> ((3 - i) * 8);
		printf(\"%s%s\", bit_str[(toprint >> 4) & 0xF], bit_str[toprint & 0x0F]);
	}");
}
*/
/*void print_u32_binary(u32 val)
{
	for (u32 i = 0; i < 4; i++)
	{
		u32 toprint = val >> ((3 - i) * 8);
		printf("%s%s", bit_str[(toprint >> 4) & 0xF], bit_str[toprint & 0x0F]);
	}
}*/