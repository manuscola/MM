#ifndef __BUDDY_H__
#define __BUDDY_H__
   
struct buddy_pool* buddy_create(unsigned int order,unsigned int min_order);
void buddy_destroy(struct buddy_pool* self);
void* buddy_malloc(struct buddy_pool* self,int size);
void buddy_free(struct buddy_pool* self,char* pointer);
int buddy_size(struct buddy_pool* self,char* pointer);
int buddy_dump(struct buddy_pool* self);
#endif
