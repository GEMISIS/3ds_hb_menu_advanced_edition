#include "asmdata.h"



/*===================================================================================
*   Data sorting/retrieving for jumps, variable declarations, and cpu operations
*
*   Uses qsort and binary search algorithms for keeping track of data in
*   a dynamically sized array of ASMData type
*
===================================================================================*/

static inline void swap_internal(bagByte *a, bagByte *b, size_t size){
    if (a == b)
        return;

    bagByte t;
    while (size--){
        t = *a;
        *a++ = *b;
        *b++ = t;
    }
    return;
}

static void qsort_internal(bagByte *begin, bagByte *end, size_t size, int(*compar)(const void *, const void *)){
    if (end <= begin) return;

    bagByte *pivot = begin;
    bagByte *l = begin + size, *r = end;


    while (l < r){
        int result = compar(l, pivot);

        if (result > 0){
            l += size;
            __builtin_prefetch(l);
        }
        else if(result < 0){
            r -= size;
            __builtin_prefetch(r);
            swap_internal(l, r, size);
        }
        else
            break;
    }

    l -= size;
    swap_internal(begin, l, size);
    qsort_internal(begin, l, size, compar);
    qsort_internal(r, end, size, compar);
    return;
}

static void _BAG_Qsort(void *data, size_t dataCount,
                        size_t dataSize, int(*compar)(const void *, const void *)){

    qsort_internal((bagByte *)data, data+dataCount*dataSize, dataSize, compar);
    return;
}


static int compareSort(const void *dest, const void *src){
    ASMData *destObj = (ASMData*)dest;
    ASMData *srcObj = (ASMData*)src;

    char *dest_str = (char*)destObj->label,
         *src_str = (char*)srcObj->label;

    return -BASM_strcmp(dest_str, src_str);
}

//case insensitive compare
static int compareSortNoCase(const void *dest, const void *src){
    ASMData *destObj = (ASMData*)dest;
    ASMData *srcObj = (ASMData*)src;

    char *dest_str = (char*)destObj->label,
    *src_str = (char*)srcObj->label;

    return -strcasecmp(dest_str, src_str);
}



static bagWord _BAG_binSearch(const void *data,  bagWord dataCount, bagWord dataSize, void *searchVal, int(*compare)(const void *, const void*)){
    bagWord min = 0, max = dataCount - 1,
             mid = 0, cmp = 0;

    if(max < 0)
        return -2;

    do{
        mid = (min + max) >> 1;
        cmp = compare((void*)data + (mid * dataSize), searchVal);

        if(cmp == 0)
            return mid;
        else if(cmp < 0)
            min = mid + 1;
        else if(cmp > 0)
            max = mid - 1;

    }while(min <= max);

    return -1;
}

static int compareFind(const void *data, const void *cmpVal){
    ASMData *destObj = (ASMData*)data;

    char *dest_str = (char*)destObj->label,
         *src_str = (char*)cmpVal;

    //printf("%s vs. %s\n", dest_str, src_str);
    return BASM_strcmp(dest_str, src_str);
}

static int compareFindNoCase(const void *data, const void *cmpVal){
    ASMData *destObj = (ASMData*)data;

    char *dest_str = (char*)destObj->label,
    *src_str = (char*)cmpVal;

    //printf("%s vs. %s\n", dest_str, src_str);
    return strcasecmp(dest_str, src_str);
}




//clear and free all memory used by data storage
ASMData *ASMData_allocateMore(ASMData *data, bagWord *count){

    if(*count <= 0){
        *count = 0;
        data = NULL;
    }

    //we need to always allocate one more than necessary
    //bagWord oldSize = ((*count)) * sizeof(ASMData);
    (*count)++;
    bagWord newSize = ((*count)) * sizeof(ASMData);

    ASMData *tempdat = realloc(data, newSize);
    if(!tempdat)
        return NULL;

    return tempdat;
}

void ASMData_cleanAll(ASMData *data, bagWord *count){
    ASMData *lines = (ASMData *)(data);
    bagWord total = (*count);

    //make sure there is data first
    if(!lines || total == 0)
        return;

    //clear all label names
    for(int i = 0; i < total; i++){
        if(lines[i].label){
            free(lines[i].label);
        }

        lines[i].label = NULL;
        lines[i].addr = 0;
    }

    free(lines);
    *count = 0;
    return;
}



bagWord ASMData_listGet(const ASMData *data, bagWord count, const char *label){
    return _BAG_binSearch((void*)data,  count,
                            sizeof(ASMData), (char*)label, &compareFind);
}

//Operations are not case sensitive anymore
bagWord ASMData_listGetNoCase(const ASMData *data, bagWord count, const char *label){
    return _BAG_binSearch((void*)data,  count,
                          sizeof(ASMData), (char*)label, &compareFindNoCase);
}


//storing and retrieving variables declared
void ASMData_sort(ASMData *data, bagWord count){
    _BAG_Qsort((void*)data, count, sizeof(ASMData), &compareSort);

#ifdef DEBUG_OUTPUT_LISTS
    BASM_PRINT("\nSorted List\n");
    for(int i = 0; i < count; i++)
        BASM_PRINT("%s\n", data[i].label);
#endif
    return;
}

void ASMData_sortNoCase(ASMData *data, bagWord count) {
    _BAG_Qsort((void*)data, count, sizeof(ASMData), &compareSortNoCase);
}
