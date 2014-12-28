
#ifndef _ASM_DATA_
#define _ASM_DATA_

#ifdef __cplusplus
extern "C" {
#endif

#include "bagasmi.h"


extern ASMData *ASMData_allocateMore(ASMData *data, bagWord *count);
extern bagWord ASMData_listGet(const ASMData *data, bagWord count, const char *label);
extern bagWord ASMData_listGetNoCase(const ASMData *data, bagWord count, const char *label);
extern void ASMData_cleanAll(ASMData *data, bagWord *count);

extern void ASMData_sort(ASMData *data, bagWord count);
extern void ASMData_sortNoCase(ASMData *data, bagWord count);

#ifdef __cplusplus
}
#endif


#endif
