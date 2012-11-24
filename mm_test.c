#include<stdio.h>
#include"mm.h"
#include<unistd.h>
#include<pthread.h>

extern int UB_size[UB_TYPES];
void * work_thread()
{
    
    int i= 0;
    int choose;
    int size;
    char* ptr[1024] = {NULL,};
    for(i = 0 ; i<1024;i++)
    {
        choose  = random()%(UB_TYPES + 1);
        if(choose == UB_TYPES)
        {
            size = random()%(1024*1024) +1;
        }
        else
        {
            size = UB_size[choose];
        }

        ptr[i]  = MALLOC(size); 
        if(ptr[i] == NULL)
        {
            fprintf(stderr,"i = %d,malloc %d  %s \n",i,size,(ptr!=NULL)?"ok":"failed");
            break;
        }
        memset(ptr[i],0 ,size);
        if(i < 10 )
            continue;
        else
        {

            FREE(ptr[i-10]);
        }

    }

   
}

int main()
{
    int ret = mm_init();
    if(ret < 0)
    {
        fprintf(stderr,"init mm failed\n");
        return -1;
    }
    pthread_t tid[10] = {0};
    int i;
    for(i = 0 ; i< 10 ; i++)
    {
        ret = pthread_create(&tid[i],NULL,work_thread,NULL);
        if(ret < 0)
        {
            fprintf(stderr,"create thread %d failed\n",i);
            return -2;
        }
    }

    for(i = 0;i < 10 ; i++)
    {
        pthread_join(tid[i], NULL);
    }

    UB_check();
    mm_destroy();
}
