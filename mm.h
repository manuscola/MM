#ifndef __MM_H__
#define __MM_H__

#define UB_TYPES 6 
#define MAX_ORDER  27
#define MIN_ORDER  5

int mm_init();
void* MALLOC(int size);
void FREE(void* ptr);

#endif
