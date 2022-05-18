#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdint.h>

// generic linked list implementation

struct ListNode;

typedef struct ListNode {
    void*               data;
    struct ListNode*    next;
    struct ListNode*    prev;
} ListNode;

typedef struct List {
    ListNode*   head;
    ListNode*   tail;
    uint32_t    count;
} List;

typedef enum {
    kNoAction   = 0,
    kRemove     = 1
} ListNodeAction;

typedef ListNodeAction (*iterator_callback)(uint32_t index, void* data);

typedef struct ListManager {
    List    (*init)     (void);
    void    (*add)      (List* list, void* data);
    int     (*remove)   (List* list, uint32_t index);
    int     (*pop)      (List* list, void** data_ptr);
    int     (*dequeue)  (List* list, void** data_ptr);
    void    (*iterate)  (List* list, iterator_callback callback);
    int     (*at)       (List* list, uint32_t index, void** data_ptr);
    void    (*destroy)  (List*);
} ListManager;

const ListManager* get_list_manager(void);

#endif
