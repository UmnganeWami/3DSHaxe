#include "Main.h"

#include "3ds.h"
#include "stdio.h"
#include "stdlib.h"

void _Main::Main_Fields_::main() {
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	printf("haiiii \n");
	printf("haiiii \n");
	printf("haiiii \n");

	while(aptMainLoop()) {
		hidScanInput();
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	};

	gfxExit();
}
