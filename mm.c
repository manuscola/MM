#include"list.h"
#include"buddy.h"
#include"mm.h"
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<pthread.h>

typedef struct UB{
    struct list_head head;
    int idx;
}UB;

typedef struct UB_pool
{
    struct list_head freelist;
    pthread_mutex_t fl_lock;
    int UB_size;
    char* begin;
    char* end;
    int total_num;
    int free_num;

}UB_pool;


int UB_size[UB_TYPES] = {1792,1024,512,256,128,64} ;
int UB_num[UB_TYPES] = {10240,10240,10240,10240,10240,10240};
struct UB_pool g_UBPool[UB_TYPES] = {0};
struct UB* p_UB[UB_TYPES] ; 
struct buddy_pool* g_buddypool = NULL;

int init_UB_pool()
{
    int i = 0;
    int j = 0;
    memset(g_UBPool,0,UB_TYPES*sizeof(struct UB_pool));
    memset(p_UB,0,UB_TYPES*sizeof(struct UB*));
    for(i = 0;i<UB_TYPES;i++)
    {
         INIT_LIST_HEAD(&(g_UBPool[i].freelist));
         pthread_mutex_init(&g_UBPool[i].fl_lock,NULL);
         g_UBPool[i].UB_size = UB_size[i];
         g_UBPool[i].total_num = UB_num[i];
         g_UBPool[i].free_num = UB_num[i];
         g_UBPool[i].begin = (char*)malloc(UB_size[i]*UB_num[i]); 
         if(g_UBPool[i].begin == NULL)
         {
             goto err_out;
         }
         g_UBPool[i].end = g_UBPool[i].begin + UB_size[i]*UB_num[i];
         p_UB[i] = (struct UB*) malloc(UB_num[i]*sizeof(UB));
         if(p_UB[i] == NULL)
         {
             goto err_out;
         }

         for(j = 0;j<UB_num[i]; j++)
         {
             list_add_tail(&(p_UB[i][j].head),&g_UBPool[i].freelist);
             p_UB[i][j].idx = j;
         }
    }
    return 0;
err_out:
    for(;i >= 0;i--)
    {
        if(p_UB[i])
            free(p_UB[i]);
        if(g_UBPool[i].begin)
            free(g_UBPool[i].begin);
    }
    return -1;
}

void* UB_malloc(int size,int idx)
{
    assert(idx >= 0 && idx < UB_TYPES);
    
    pthread_mutex_lock(&g_UBPool[idx].fl_lock);
    
    if(list_empty(&(g_UBPool[idx].freelist)))
    {
        return NULL;
    }

    struct UB* ub =(UB*) (g_UBPool[idx].freelist.next);
    list_del_init((g_UBPool[idx].freelist.next));
    g_UBPool[idx].free_num--;

    pthread_mutex_unlock(&g_UBPool[idx].fl_lock);
    
    int j = ub->idx;
    assert(j >= 0 && j <UB_num[idx]);

    return g_UBPool[idx].begin+j*g_UBPool[idx].UB_size;

}
int mm_init()
{
    int ret = init_UB_pool();
    if(ret < 0 )
    {
        fprintf(stderr,"init UB pool failed\n");
        return -1;
    }
    g_buddypool = buddy_create(MAX_ORDER,MIN_ORDER);
    if(g_buddypool == NULL)
    {
        fprintf(stderr,"init buddy pool failed\n");
        return -2;
    }
    return 0;
}
void * MALLOC(int size)
{
    int idx = -1;
    int j;
    for(j = 0 ;j < UB_TYPES;j++)
    {
        if(size == UB_size[j])
        {
            idx = j;
            break;
        }
    }
    if(idx == -1)
    {
        return (void *) buddy_malloc(g_buddypool,size);
    }
    else
    {
        void* ret_ptr = UB_malloc(size,idx);
        if(ret_ptr == NULL)
        {
            return buddy_malloc(g_buddypool,size);
        }    
        else
            return ret_ptr;
    }
}
void UB_free(void* ptr,int idx)
{
    assert(idx >=0 && idx < UB_TYPES);

    int j = ((unsigned long)ptr -(unsigned long) g_UBPool[idx].begin)/g_UBPool[idx].UB_size;
    assert(j < UB_num[idx]);

    struct UB* ub =&(p_UB[idx][j]);
    pthread_mutex_lock(&g_UBPool[idx].fl_lock);
    list_add_tail(&(ub->head),&g_UBPool[idx].freelist);
    g_UBPool[idx].free_num++;
    pthread_mutex_unlock(&g_UBPool[idx].fl_lock);

}
void FREE(void* ptr)
{
    int idx  = -1;
    int j = 0; 

    assert(ptr != NULL);

    for(j = 0;j<UB_TYPES;j++)
    {
        if((unsigned long)ptr >= (unsigned long)g_UBPool[j].begin && (unsigned long)ptr < (unsigned long )g_UBPool[j].end )
        {
            idx = j;
            break;
        }
    }
    if(idx == -1)
    {
        buddy_free(g_buddypool,ptr);
    }
    else
    {
        UB_free(ptr,idx);
    }
}

int UB_check()
{

    int count;
    struct list_head* next = NULL;
    int idx,j;
    for(idx = 0; idx< UB_TYPES;idx++)
    {
        count = 0;
        
        pthread_mutex_lock(&g_UBPool[idx].fl_lock);

        next = g_UBPool[idx].freelist.next;
        while(next != &(g_UBPool[idx].freelist))
        {
            count++;
            next = next->next;
        }

        pthread_mutex_unlock(&g_UBPool[idx].fl_lock);
        if(count != g_UBPool[idx].free_num)
        {
           fprintf(stderr,"UB pool corrupted ,idx = %d\n,free_num = %d, but ub in free list is %d\n",idx, g_UBPool[idx].free_num,count);
        }
    }
}

int UB_destroy(struct UB_pool* g_UBPool)
{
   int i ;
   for(i = 0;i<UB_TYPES;i++)
   {
       free(p_UB[i]);
       free(g_UBPool[i].begin);
   }
   return 0;
}
int mm_destroy()
{
    buddy_destroy(g_buddypool);
    UB_destroy(g_UBPool);
}
