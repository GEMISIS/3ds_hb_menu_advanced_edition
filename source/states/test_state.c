#include "test_state.h"
#include "hb_menu.h"

#include <3ds.h>
#include <stdio.h>

exitStatus_t testStateInit()
{
	consoleInit(GFX_TOP, NULL);

	printf("Inited!");
	return CHANGED_STATE;
}

exitStatus_t testStateUpdate()
{
	if(hidKeysDown() & KEY_START)
	{
		GotoHBMenu();
		return CHANGED_STATE;
	}
	return NONE;
}

exitStatus_t testStateRender()
{
	return NONE;
}

exitStatus_t testStateDestroy()
{
	printf("Destroyed!");
	return NONE;
}

exitStatus_t testStatePostExit()
{
	return NONE;
}

exitStatus_t SetTestState()
{
	if(core_state.Destroy != NULL)
	{
		core_state.Destroy();
	}

	core_state.Initialize = testStateInit;
	core_state.Update = testStateUpdate;
	core_state.Render = testStateRender;
	core_state.Destroy = testStateDestroy;
	core_state.PostExit = testStatePostExit;

	if(core_state.Initialize != NULL)
	{
		return core_state.Initialize();
	}
	return NONE;
}