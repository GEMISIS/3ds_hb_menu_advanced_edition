/*---------------------------------------------------------------------------------
 BAGASM  Copyright (C) 2011 - 2012
  BassAceGold - <BassAceGold@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.

  3. Restricting the usage of or portions of code in this library
     from other software developers and or content creators
     is strictly prohibited in applications with software creation capabilities
     and or with the intent to profit from said restrictions.

  4. This notice may not be removed or altered from any source
     distribution.
---------------------------------------------------------------------------------*/

#include "../../core/bagasmi.h"
#include "../../core/scriptstack.h"
#include <3ds.h>

//extern unsigned time(time_t *);
const char Platform[] = "3DS :D";


extern void waitExit(void);

void waitExit(void){
  exit(0);
}

void Timer_start(uint32_t *increment){
 // *increment = getSysTime();
}

float Timer_getTicks(uint32_t *increment){
  /*u32 finalTime = getSysTime() - *increment;
  return (finalTime * 49.667)/1000000;*/
  return 0.0;
}


int getTime(char *array){
  /*struct rtc time;
  ds2_getTime(&time);
  int i = 0;
  array[i] = time.seconds;i++;
  array[i] = time.minutes;i++;
  array[i] = time.hours;i++;
  array[i] = time.weekday;i++;
  array[i] = time.day;i++;
  array[i] = time.month;i++;
  array[i] = time.year;*/
  return 1;
}


//get launched path
int main(int argc, char *argv[]){
  srvInit(); //needed for everything
  aptInit(); //needed for everything
  hidInit(NULL); //needed for input
  gfxInit(); //makes displaying to screen easier
  fsInit(); //needed for filesystem stuff
  sdmcInit();

  consoleInit(GFX_BOTTOM, NULL);

  printf("Starting...");
  gspWaitForVBlank();
  gfxFlushBuffers();
  gfxSwapBuffers();

  char *filename = "helloworld.asm";

  FilePath *prgmPath = FilePath_Init();
  FilePath_Convert(prgmPath, filename, strlen(filename));

  System_ScriptsInit();
  if(System_PushScript(prgmPath, NULL, 0, 0, 0)) {
      while(System_ScriptsRunning()) System_ProcessScripts(0);
      while (aptMainLoop())
      {
        gspWaitForVBlank();
        hidScanInput();

        // Your code goes here

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
          break; // break in order to return to hbmenu

        // Example rendering code that displays a white pixel
        // Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
        u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
        memset(fb, 0, 240*400*3);
        fb[3*(10+10*240)] = 0x00;
        fb[3*(10+10*240)+1] = 0xFF;
        fb[3*(10+10*240)+2] = 0x00;

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
      }
  } else {
    printf("failed to push script!\n");
     while (aptMainLoop())
      {
        gspWaitForVBlank();
        hidScanInput();

        // Your code goes here

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
          break; // break in order to return to hbmenu

        // Example rendering code that displays a white pixel
        // Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
        u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
        memset(fb, 0, 240*400*3);
        fb[3*(10+10*240)] = 0xFF;
        fb[3*(10+10*240)+1] = 0x00;
        fb[3*(10+10*240)+2] = 0x00;

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
      }
  }

  FilePath_Destroy(prgmPath);

  //closing all services even more so
  sdmcExit();
  fsExit();
  gfxExit();
  hidExit();
  aptExit();
  srvExit();

  return 0;
}

