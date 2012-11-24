#include<stdio.h>
#include"buddy.h"
#include<stdlib.h>
#include<time.h>
int main()
{
    int order = 24;
    struct buddy_pool* pool = buddy_create(order,5);
    if((unsigned long)pool > (unsigned long) (-1000) ) // errcode can not bigger than 1000
    {
        fprintf(stderr," malloc failed ,return %lx\n",(unsigned long)pool);
        return -1;
    }
    int size = 0;
    int i = 0;
    char* ptr[12] ;
    srand(time(NULL));
    for(;i <12 ; i++)
    {
        size = random()%(1024*1024);
        ptr[i] = buddy_malloc(pool,size);
        fprintf(stderr,"i = %d , size = %d, ptr[i] = %lx\n ",i,size,(unsigned long)ptr[i]);
    }
    fprintf(stderr,"=====================================\n");
    buddy_dump(pool);
    for(i = 0; i < 12;i= i+2)
    {
        if(ptr[i] != NULL)
        {
            buddy_free(pool,ptr[i]);
            ptr[i] = NULL;
        }
    }
    buddy_dump(pool);
    buddy_destroy(pool);
    return 0;
}
