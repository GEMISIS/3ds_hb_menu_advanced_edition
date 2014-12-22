#include "bagasmi.h"
#include "scriptstack.h"

#if defined(USE_SCRIPT_STACK)

static char _script_exts[2][6] = {
    "basm", "asm",
};

static ASMProcess_Stack Scripts;


/*=============================================================================
Individual Script Handling wrappers
=============================================================================*/
//general script loading and closing functions
static void _closeScript(ASMSys *sys){
    if(!sys) return;
    ASM_CleanSystem(sys);
    return;
}

static ASMSys *_openScript(FilePath *file, char **argv, int argc, va_list *vl){
    if(!file){
        BASM_DBGPRINT("Failed to load script: Missing file\n");
        return NULL;
    }

    Scripts.memUsed = ASM_MEMUSED;//grab current used memory incase of errors


    //check for extension
    char *fileExt = FilePath_FileExt(file);

    if(fileExt && ( !strncasecmp(fileExt, _script_exts[0], strlen(fileExt)) ||
                    !strncasecmp(fileExt, _script_exts[1], strlen(fileExt))
    )){

    }
    else{
        file->folder = 0;//mark this path as a file
        //if no extension given, then auto find it
        //priority load .basm files ahead of .asm files
        char *newFile = NULL;
        for(int i = 0; i < 2; i++){
            FilePath_SwapExt(file, _script_exts[i]);
            newFile = FilePath_Str(file);

            struct stat st;
            if ( lstat(newFile, &st) >= 0 ) break;
            else {
                FilePath_DelStr(file);
                newFile = NULL;
            }

        }
        //if no file with the given extensions were found, error out
        if(!newFile) return NULL;
    }
    FilePath_DelStr(file);


    ASMSys *newSys = calloc(1, sizeof(ASMSys));
    if(!newSys){
        FilePath_DelStr(file);
        BASM_DBGPRINT("Error allocating script system");
        return NULL;
    }

    if(argc > 0) {
        if(!argv && vl){
            ASM_SetArgsV(newSys, argc, vl);
        }
        else if(argv && !vl) {
            ASM_InitARGV(newSys, argc, argv);
        }
    }

    if(ASM_InitSystem(file, newSys, ASM_DATAPURGE | ASM_JUMPPURGE) != 1){
        _closeScript(newSys);
        free(newSys);

        newSys = NULL;
        ASM_MEMUSED = Scripts.memUsed;
        return NULL;
    }

    return newSys;
}



int System_CompileScript(FilePath *file){
    ASMSys *newSys = calloc(1, sizeof(ASMSys));
    if(!newSys){
        BASM_DBGPRINT("Error allocating script system");
        return 0;
    }

    int stat = ASM_BuildFile(newSys, file);
    free(newSys);
    newSys = NULL;
    return stat;
}

/*=============================================================================
Script stack management
=============================================================================*/
//Any errors with scripts should be handled gracefully!
static void _processClean(ASMProcess *tmp){
    tmp->mode = 0;
    tmp->child = 0;
    tmp->parent = NULL;

    //close script
    _closeScript(tmp->process);
    free(tmp->process);
    tmp->process = NULL;

    //free memory of old script
    free(tmp->name);
    tmp->name = NULL;

    //free script slot
    free(tmp);
    return;
}



static ASMProcess *_spop(int pos){
    if( pos > Scripts.loaded)
        return NULL;

    ASMProcess *rmScript = Scripts.list[pos];
    Scripts.loaded--;
    if( Scripts.loaded < 0 ) Scripts.loaded = 0;


    for(int i = pos; i < Scripts.loaded; i++) Scripts.list[i] = Scripts.list[i + 1];
    for(int i = Scripts.loaded; i < SCRIPT_STACK_MAXSIZE; i++) Scripts.list[i] = NULL;
    return rmScript;
}

static int _spush(ASMProcess *newScript){
    //first check if stack is full
    if(Scripts.loaded >= SCRIPT_STACK_MAXSIZE - 1)
        return 0;

    Scripts.loaded++;
    //now that list is embiggened, we can shift all the scripts down
    for(int i = Scripts.loaded; i >= 1; i--) {
        Scripts.list[i] = Scripts.list[i - 1];
    }
    Scripts.list[0] = newScript;
    return 1;
}


static char scriptByName(const char *file){
    for (int i = 0; i < Scripts.loaded; i++) {
        if(!strcmp(file, Scripts.list[i]->name))
            return i;
    }
    return -1;//not found
}



//from ds2_main.c using HomeMenu
/*extern int ForceQuitScript(void);
#define EXIT_FRAME_COUNT (60*2)

static int _checkForceQuit(ASMSys *sys, int *counter){
    //force exit script if you can't do so through the menu
    if(!sys->settings.passive && sys->video.update){
        sys->video.update = 0;

        (*counter) += Pad.Held.Select;
        (*counter) *= Pad.Held.Select;//should reset to 0 when not held
        if((*counter) >  EXIT_FRAME_COUNT && ForceQuitScript()){
            sys->cpu.exit = BASM_EXIT_CLEAN;
            return 1;
        } else if ((*counter) > EXIT_FRAME_COUNT){
            (*counter) = 0;
        }
    }
    return 0;
}
*/


static int _runScript(int scriptNum){

    ASMSys *sys = Scripts.list[scriptNum]->process;
    if(!sys) return 0;

    //printf("running script: %s\n", Scripts.list[scriptNum]->name);
    while(ASM_Step(sys)){
        //if(_checkForceQuit(sys, &counter)) break;
    }

    //check if we need to unload the script
    int result = sys->cpu.exit;

    if(result != BASM_EXIT_ERROR && Scripts.list[scriptNum]->mode & SCRIPT_C) {
        //reset cached scripts
        ASM_ResetSys(sys);
        return 0;
    }

    else if(result == BASM_EXIT_CLEAN || result == BASM_EXIT_ERROR){
        //cleanly exit script and remove froms stack
        return 1;
    }
    else if(result == BASM_EXIT_SUSPEND) //yielding to menu
        sys->cpu.exit = 0; //reset exit code

    return 0;
}




/*====================================
Public Script Stack API
=====================================*/

void System_ScriptsInit(void){
    //memset(&Scripts, 0, sizeof(ASMProcess_Stack));
    for(int i = 0; i < SCRIPT_STACK_MAXSIZE; i++){
        Scripts.list[i] = NULL;
    }
    Scripts.loaded = 0;
}

void System_SwapScripts(int src, int dest){
    ASMProcess *temp = Scripts.list[dest];
    Scripts.list[dest] = Scripts.list[src];
    Scripts.list[src] = temp;
}


//TODO: Fix reduce duplicate loading code in push script and push script 2
//push a script onto the processing stack
int System_PushScript(FilePath *file, ASMSys *parentScript, int mode, char **argv, int argc, ...){
    //grab script name
    char *name = FilePath_FileName(file);
    //if we are pushing a script to be cached, check if its loaded already
    if(mode & SCRIPT_C) {
        int testNum = scriptByName(name);

        //if script has been loaded already, update its arguments
        if(testNum > -1) {
            if(!argv && argc){
                va_list vl;
                va_start(vl, argc);
                ASM_SetArgsV(Scripts.list[testNum]->process, argc, &vl);
                va_end(vl);
            } else if (argv && argc){
                ASM_InitARGV(Scripts.list[testNum]->process, argc, argv);
            }

            //set script to top priority if it already isn't
            if(testNum != 0){
                //printf("reprioritizing scripts\n");
                ASMProcess *temp = _spop(testNum);
                _spush(temp);
            }
            return 1;
        }
    }

    ASMProcess *newScript = calloc(sizeof(ASMProcess), 1);
    //BAG_DBG_ASSERT(newScript);
    if(!newScript){
        BASM_DBGPRINT("Error allocating script system");
        return 0;
    }

    //allocate space for script name
    newScript->name = calloc(sizeof(char), strlen(name) + 1);
    if(!newScript->name){
        BASM_DBGPRINT("Error allocating script name: %d\n", Scripts.loaded);
        free(newScript);
        return 0;
    }
    strncpy(newScript->name, name, strlen(name));

    //set script mode
     newScript->mode = mode;

    //then finally load the script
    if(argv && argc){
        newScript->process = _openScript(file, argv, argc, NULL);
    } else if (!argv && argc) {
        va_list vl;
        va_start(vl, argc);
        newScript->process = _openScript(file, NULL, argc, &vl);
        va_end(vl);
    } else if (!argc) {
        newScript->process = _openScript(file, NULL, 0, NULL);
    }

    //BAG_DBG_ASSERT(newScript->process);
    //if the script fails to allocate, or there is no room on the stack, cleanup the script
    if(!newScript->process || !_spush(newScript)) {
        _processClean(newScript);
        newScript = NULL;
        return 0;
    }

    //when the script is successful, if it is a child script of another
    //script, keep track of the parent script
    if(parentScript) {
        newScript->parent = parentScript;
        newScript->child = 1;
    }

    return 1;
}

//remove a script from the processing stack
void System_PopScript(int pos) {
    if( pos > Scripts.loaded){
        printf("invalid pop position!\n");
        return;
    }

    //get the script to remove
    ASMProcess *rmScript = _spop(pos);

    //if script is child, copy the childs v0 and v1 register results
    //to the parent. Basically, the child script can return values to the parent script
    if(rmScript->child) {
        if(rmScript->parent) {
            ASMSys *parent = rmScript->parent;
            ASMSys *child = rmScript->process;
            parent->cpu.reg[REG_V0] = child->cpu.reg[REG_V0];
            parent->cpu.reg[REG_V1] = child->cpu.reg[REG_V1];
        }
    }

    if (!rmScript){
        printf("error popping script: %d\n", pos);
        return;
    }
    _processClean(rmScript);
    return;
}


void System_ProcessScripts(int mode) {
    Scripts.stackPointer = 0;
    for(; Scripts.stackPointer < Scripts.loaded; Scripts.stackPointer++){
        int *i = &Scripts.stackPointer;

        //make sure we aren't trying to process scripts that don't exist...
        if(!Scripts.list[*i]) break;

        int val = _runScript(*i);
        //if a value is returned, the script has exited and needs to be removed from the stack
        if(val) {
            ASMSys *parent = Scripts.list[*i]->parent;
            int scriptMode = Scripts.list[*i]->mode;
            System_PopScript(*i);

            //if return to parent mode, find parent script in stack
            if(scriptMode & SCRIPT_RETURNTOPARENT && parent != NULL){
                int newStart = 0;
                for(; newStart < Scripts.loaded; newStart++){
                    if(Scripts.list[newStart]->parent == parent)break;
                }
                //once parent script is found, set stack pointer to continue running
                //the parent script
                *i = newStart;
            }

            (*i)--;
        }
    }
}

void System_RunCached(void){
    if(!Scripts.list[0]) return;
    _runScript(0);
}

int System_ScriptsRunning(void){
    return Scripts.loaded;
}

char *System_GetScriptName(int number){
    if(number < 0 || number >= Scripts.loaded)
        return NULL;

    return Scripts.list[number]->name;
}

ASMSys *System_GetScript(int number){
    if(number < 0 || number >= Scripts.loaded)
        return NULL;
    return Scripts.list[number]->process;
}

//Resets the script stack pointer back to 0 on the next processing of scripts
void System_TriggerReprocessStack(void) {
    Scripts.stackPointer = -1;
}


int System_GetScriptMode(int number){
    if(!Scripts.list[number] || number < 0 || number >= Scripts.loaded) return -1;
    return Scripts.list[number]->mode;
}

void System_CloseAllScripts(void){
    while(System_ScriptsRunning()) System_PopScript(0);
    return;
}

#endif
