#pragma once

typedef enum
{
	NONE = 0,
	EXIT_ERROR = -1,
	CHANGED_STATE = -2,
	EXIT_NORMAL = 1
}exitStatus_t;

typedef exitStatus_t (*InitializeCallback)(void);
typedef exitStatus_t (*UpdateCallback)(void);
typedef exitStatus_t (*RenderCallback)(void);
typedef exitStatus_t (*DestroyCallback)(void);
typedef exitStatus_t (*PostExitCallback)(void);

typedef struct
{
	InitializeCallback Initialize;
	UpdateCallback Update;
	RenderCallback Render;
	DestroyCallback Destroy;
	PostExitCallback PostExit;
}state_t;

extern state_t core_state;