#include "linked_list.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ds_err_t llist_init(linked_list_s **llist_out, size_t elem_size){
    linked_list_s *llist;
    if ((llist = calloc(1, sizeof(linked_list_s))) == NULL){
        fprintf(stderr, "Failed to Allocate Memory for Linked List...");
        return DS_ERR_ALLOC;
    }

    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    llist->elem_size = elem_size;

    *llist_out = llist;

    return DS_OK;
}

ds_err_t llist_push_front(linked_list_s *llist, void *element){
    node_s *new_node;
    if ((new_node = calloc(1, sizeof(node_s))) == NULL){
        fprintf(stderr, "Failed to Allocate Memory for New Node...");
        return DS_ERR_ALLOC;
    }

    if ((new_node->data = calloc(1, llist->elem_size)) == NULL){
        fprintf(stderr, "Failed to Allocate Memory for Node Data...");
        free(new_node);
        return DS_ERR_ALLOC;
    }
    
    memcpy(new_node->data, element, llist->elem_size);
    new_node->next = llist->head;
    llist->head = new_node;
    if (llist->size == 0) {
        llist->tail = new_node;
    }

    llist->size += 1;

    return DS_OK;
}

ds_err_t llist_push_back(linked_list_s *llist, void *element){
    node_s *new_node;
    if ((new_node = calloc(1, sizeof(node_s))) == NULL){
        fprintf(stderr, "Failed to Allocate Memory for New Node...");
        return DS_ERR_ALLOC;
    }

    if ((new_node->data = calloc(1, llist->elem_size)) == NULL){
        fprintf(stderr, "Failed to Allocate Memory for New Node...");
        free(new_node);
        return DS_ERR_ALLOC;
    }

    new_node->next = NULL;  
    memcpy(new_node->data, element, llist->elem_size);

    if (llist->size == 0){
        llist->head = new_node;
        llist->tail = new_node;
    } else {
        llist->tail->next = new_node;
        llist->tail = new_node;
    }

    llist->size += 1;
    return DS_OK;
}

ds_err_t llist_pop_front(linked_list_s *llist, void *element_out){
    if (llist->size == 0){
        fprintf(stderr, "Can't Pop from an Empty List...");
        return DS_ERR_EMPTY;
    }

    node_s *old_head = llist->head;
    llist->head = llist->head->next;
    // When NULL, copying is discarded, and the element is removed
    if (element_out != NULL) memcpy(element_out, old_head->data, llist->elem_size);
    free(old_head->data);
    free(old_head);

    if (llist->size == 1) llist->tail = NULL;

    llist->size -= 1;
    return DS_OK;
}

ds_err_t llist_pop_back(linked_list_s *llist, void *element_out){
    if (llist->size == 0){
        fprintf(stderr, "Can't Pop from an Empty List...");
        return DS_ERR_EMPTY;
    }   

    if (llist->size == 1){
        if (element_out != NULL) memcpy(element_out, llist->head->data, llist->elem_size);
        free(llist->head->data);
        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
    } else {
        node_s *current;
        for (current=llist->head; current->next->next!=NULL; current=current->next);
        if (element_out != NULL) memcpy(element_out, current->next->data, llist->elem_size);
        free(current->next->data);
        free(current->next);
        current->next = NULL;
        llist->tail = current;
    }

    llist->size -= 1;
    return DS_OK;
}

ds_err_t llist_get(const linked_list_s *llist, size_t index, void *element_out){
    if (index >= llist->size) return DS_ERR_OUT_OF_BOUNDS;

    node_s *current;
    size_t i;
    for (i=0, current=llist->head; i<index; i++, current=current->next);
    memcpy(element_out, current->data, llist->elem_size);

    return DS_OK;
}

ds_err_t llist_set(linked_list_s *llist, void *element, size_t index){
    if (index >= llist->size) return DS_ERR_OUT_OF_BOUNDS;

    node_s *current;
    size_t i;
    for (i=0, current=llist->head; i<index; i++, current=current->next);
    memcpy(current->data, element, llist->elem_size);

    return DS_OK;
}

ds_err_t llist_insert(linked_list_s *llist, void *element, size_t index){
    if (index > llist->size) return DS_ERR_OUT_OF_BOUNDS;

    if (index == 0) return llist_push_front(llist, element);
    if (index == llist->size) return llist_push_back(llist, element);
    
    node_s *new_node;
    if ((new_node = calloc(1, sizeof(node_s))) == NULL) return DS_ERR_ALLOC;

    if ((new_node->data = calloc(1, llist->elem_size)) == NULL){
        free(new_node);
        return DS_ERR_ALLOC;
    }

    memcpy(new_node->data, element, llist->elem_size);

    node_s *current;
    size_t i;
    for (i=0,current=llist->head;i<index-1;i++,current=current->next);
    new_node->next = current->next;
    current->next = new_node;
    llist->size += 1;

    return DS_OK;
}

ds_err_t llist_foreach(linked_list_s *llist, llist_foreach_fn fn, void *user_data){
    if ((llist == NULL) || (fn == NULL)) return DS_ERR_INVALID_ARGUMENT;

    node_s *current = llist->head;
    size_t index = 0;
    while (current){
        fn(current->data, index, user_data);
        current = current->next;
        index++;
    }
    return DS_OK;
}

ds_err_t llist_remove(linked_list_s *llist, size_t index){
    if (index >= llist->size) return DS_ERR_OUT_OF_BOUNDS;

    if (index == 0) return llist_pop_front(llist, NULL);
    if (index == llist->size - 1) return llist_pop_back(llist, NULL);

    node_s *current;
    size_t i;
    for (i=0,current=llist->head;i<index-1;i++,current=current->next);

    node_s *to_remove = current->next;
    current->next = to_remove->next;
    free(to_remove->data);
    free(to_remove);

    llist->size -= 1;
    return DS_OK;
}

ds_err_t llist_free(linked_list_s *llist){
    if (llist == NULL) return DS_OK;
    
    node_s *current = llist->head;
    node_s *next;
    while (current != NULL){
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(llist);
    return DS_OK;
}