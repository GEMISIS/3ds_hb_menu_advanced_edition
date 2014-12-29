#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "filesystem.h"
#include "state.h"

#include "hb_menu.h"
#include "test_state.h"

state_t core_state;

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

int main()
{
	srvInit();
	aptInit();
	gfxInit();
	initFilesystem();
	openSDArchive();
	hidInit(NULL);
	irrstInit(NULL);
	acInit();
	ptmInit();
	hbInit();

	srand(svcGetSystemTick());

	exitStatus_t exitStatus = NONE;

	core_state.Initialize = NULL;
	core_state.Update = NULL;
	core_state.Render = NULL;
	core_state.Destroy = NULL;
	core_state.PostExit = NULL;

	exitStatus = GotoHBMenu();

	while(aptMainLoop())
	{
		hidScanInput();

		if(exitStatus == EXIT_ERROR || exitStatus == EXIT_NORMAL)
		{
			break;
		}

		if(core_state.Update != NULL)
		{
			exitStatus = core_state.Update();
		}
		if(exitStatus == EXIT_ERROR || exitStatus == EXIT_NORMAL)
		{
			break;
		}

		if(exitStatus != CHANGED_STATE)
		{
			if(core_state.Render != NULL)
			{
				exitStatus = core_state.Render();
			}

			gfxFlushBuffers();
			gfxSwapBuffers();

			gspWaitForVBlank();
		}
	}

	if(core_state.Destroy != NULL)
	{
		core_state.Destroy();
	}

	// cleanup whatever we have to cleanup
	hbExit();
	ptmExit();
	acExit();
	irrstExit();
	hidExit();
	gfxExit();
	exitFilesystem();
	closeSDArchive();
	aptExit();
	srvExit();

	if(core_state.PostExit != NULL)
	{
		core_state.PostExit();
	}

	return 0;
}
