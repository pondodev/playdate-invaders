#include "list.h"

static ListNode* node_at(List* list, uint32_t index) {
    if (list == NULL || list->count == 0) return NULL;
    // decide if we seek from the start or the end
    uint32_t halfway = list->count / 2;
    ListNode* node = NULL;
    if (index > halfway) { // seek from end
        node = list->tail;
        for (int i = list->count - 1; i > index; --i) {
            node = node->prev;
        }
    } else { // seek from start
        node = list->head;
        for (int i = 0; i < index; ++i) {
            node = node->next;
        }
    }

    return node;
}

static List init(void) {
    return (List) {
        .head = NULL,
        .tail = NULL,
        .count = 0
    };
}

static void add(List* list, void* data) {
    if (list == NULL) return;

    ListNode* new_node = malloc(sizeof(ListNode));
    new_node->data = data;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
    } else {
        list->tail->next = new_node;
    }
    new_node->prev = list->tail;
    list->tail = new_node;

    list->count++;
}

static int remove(List* list, uint32_t index) {
    if (list == NULL || index >= list->count) return 0;

    ListNode* to_remove = node_at(list, index);
    if (to_remove == NULL) return 0;

    if (list->count > 1) {
        ListNode* next = to_remove->next;
        ListNode* prev = to_remove->prev;
        if (next != NULL) {
            next->prev = prev;
        } else {
            list->tail = prev;
        }

        if (prev != NULL) {
            prev->next = to_remove->next;
        } else {
            list->head = next;
        }
    } else {
        list->head = NULL;
        list->tail = NULL;
    }

    free(to_remove->data);
    free(to_remove);

    list->count--;

    return 1;
}

static int pop(List* list, void** data_ptr) {
    if (list == NULL) return 0;

    ListNode* tail_node = list->tail;
    if (list->count > 1) {
        list->tail = tail_node->prev;
        list->tail->next = NULL;
    } else {
        list->head = NULL;
        list->tail = NULL;
    }

    *data_ptr = tail_node->data;
    free(tail_node);

    list->count--;

    return 1;
}

static int dequeue(List* list, void** data_ptr) {
    if (list == NULL) return 0;

    ListNode* head_node = list->head;
    if (list->count > 1) {
        list->head = head_node->next;
    } else {
        list->head = NULL;
        list->tail = NULL;
    }

    *data_ptr = head_node->data;
    free(head_node);

    list->count--;

    return 1;
}

static void iterate(List* list, iterator_callback callback) {
    if (list == NULL) return;

    ListNode* current_head = list->head;
    uint32_t index = 0;
    while (current_head != NULL) {
        void* data = current_head->data;
        ListNodeAction action = callback(index, data);
        current_head = current_head->next;
        switch (action) {
            case kNoAction:
                ++index;
                break;

            case kRemove:
                remove(list, index);
                break;
        }
    }
}

static int at(List* list, uint32_t index, void** data_ptr) {
    if (list == NULL || index >= list->count) return 0;

    ListNode* node = node_at(list, index);
    if (node == NULL) return 0;

    *data_ptr = node->data;

    return 1;
}

static void destroy(List* list) {
    ListNode* node = list->head;
    while (node != NULL) {
        free(node->data);
        free(node);
        node = node->next;
    }
}

static const ListManager lm = {
    .init = init,
    .add = add,
    .remove = remove,
    .pop = pop,
    .iterate = iterate,
    .at = at,
    .destroy = destroy
};

const ListManager* get_list_manager() {
    return &lm;
}
