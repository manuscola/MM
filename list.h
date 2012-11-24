#ifndef _LIST_H_
#define _LIST_H_

struct list_head {  
    struct list_head *next, *prev;  
};  

inline int list_empty(struct list_head *head)
{
    return head->next == head;
}
inline void INIT_LIST_HEAD(struct list_head *list)  
{  
    list->next = list;  
    list->prev = list;  
}

static inline void __list_add(struct list_head *new,  
        struct list_head *prev,  
        struct list_head *next)  
{  
    next->prev = new;  
    new->next = next;  
    new->prev = prev;  
    prev->next = new;  
}

inline void list_add(struct list_head *new, struct list_head *head)  
{  
    __list_add(new, head, head->next);  
}  

inline void list_add_tail(struct list_head *new, struct list_head *head)  
{  
    __list_add(new, head->prev, head);  
}

inline void __list_del(struct list_head * prev, struct list_head * next)  
{  
    next->prev = prev;  
    prev->next = next;  
} 

inline void list_del_init(struct list_head *entry)  
{  
    __list_del(entry->prev, entry->next);  
    INIT_LIST_HEAD(entry);  
}

#endif
