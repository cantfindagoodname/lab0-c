#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof *head);
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL)
        return;
    element_t *elem, *safe;
    list_for_each_entry_safe (elem, safe, l, list) {
        q_release_element(elem);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *elem = malloc(sizeof *elem);
    if (elem == NULL)
        return false;
    elem->value = malloc(strlen(s) + 1);
    if (elem->value == NULL) {
        free(elem);
        return false;
    }
    memcpy(elem->value, s, strlen(s));
    elem->value[strlen(s)] = '\0';
    list_add(&elem->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *elem = malloc(sizeof *elem);
    if (elem == NULL)
        return false;
    elem->value = malloc(strlen(s) + 1);
    if (elem->value == NULL) {
        free(elem);
        return false;
    }
    memcpy(elem->value, s, strlen(s));
    elem->value[strlen(s)] = '\0';
    list_add_tail(&elem->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    struct list_head *node = head->next;
    list_del_init(node);

    if (sp != NULL) {
        size_t len = (bufsize - 1) ^
                     (((bufsize - 1) ^
                       (strlen(list_entry(node, element_t, list)->value))) &
                      -(strlen(list_entry(node, element_t, list)->value) <
                        (bufsize - 1)));
        char *psp = list_entry(node, element_t, list)->value;
        for (int i = 0; i < bufsize - 1; ++i) {
            sp[i ^ ((i ^ len) & -(i > len))] = *(psp + (i & -(i < len)));
        }
        sp[len] = '\0';
    }

    return list_entry(node, element_t, list);
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    struct list_head *node = head->prev;
    list_del_init(node);

    if (sp != NULL) {
        size_t len = (bufsize - 1) ^
                     (((bufsize - 1) ^
                       (strlen(list_entry(node, element_t, list)->value))) &
                      -(strlen(list_entry(node, element_t, list)->value) <
                        (bufsize - 1)));
        char *psp = list_entry(node, element_t, list)->value;
        for (int i = 0; i < bufsize - 1; ++i) {
            sp[i ^ ((i ^ len) & -(i > len))] = *(psp + (i & -(i < len)));
        }
        sp[len] = '\0';
    }

    return list_entry(node, element_t, list);
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    int size = 0;
    if (head == NULL || list_empty(head))
        return size;
    struct list_head *node;
    list_for_each (node, head) {
        ++size;
    }
    return size;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */

bool q_delete_mid(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return false;
    struct list_head *forward = NULL, *backward = NULL;
    for (forward = head->next, backward = head->prev; forward != backward;
         backward = backward->prev) {
        forward = forward->next;
        if (forward == backward)
            break;
    }
    list_del_init(forward);
    q_release_element(list_entry(forward, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */

// TODO remove duplicates at once instead of one at a time
bool q_delete_dup(struct list_head *head)
{
    if (head == NULL)
        return false;

    element_t *elem, *safe, *last = NULL;
    list_for_each_entry_safe (elem, safe, head, list) {
        if (&safe->list == head)
            break;
        if (!strcmp(elem->value, safe->value)) {
            last = safe;
            list_del_init(&elem->list);
            q_release_element(elem);
        } else {
            if (last != NULL && last == elem) {
                list_del_init(&elem->list);
                q_release_element(elem);
            }
        }
    }
    if (last == list_last_entry(head, element_t, list)) {
        list_del_init(&elem->list);
        q_release_element(elem);
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    int phase = 0;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        if (phase) {
            struct list_head *swap = node->prev;
            swap->prev->next = node;
            node->prev->next = safe;
            node->next = swap;
            safe->prev = swap;
            node->prev = swap->prev;
            swap->prev = node;
        }
        phase ^= 1;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        list_del(node);
        list_add(node, head);
    }
}

struct list_head *merge(struct list_head *a, struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;
    for (;;) {
        if (strcmp(list_entry(a, element_t, list)->value,
                   list_entry(b, element_t, list)->value) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (head == NULL || head->next == NULL)
        return head;

    struct list_head *left = head;
    struct list_head *right = left->next;

    while (right && right->next) {
        left = left->next;
        right = right->next->next;
    }
    right = left->next;
    left->next = NULL;

    left = merge_sort(head);
    right = merge_sort(right);

    return merge(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *start = head->next;
    struct list_head *final = head->prev;
    start->prev = final->next = NULL;
    INIT_LIST_HEAD(head);

    head->next = merge_sort(start);
    head->next->prev = head;

    struct list_head *temp = head->next;
    for (; temp->next != NULL; temp = temp->next) {
        temp->next->prev = temp;
    }
    head->prev = temp;
    temp->next = head;
}

/*
 * Shuffle elements of queue
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing
 */
void q_shuffle(struct list_head *head)
{
    int size = q_size(head);
    struct list_head *node, *end = head;
    while (size > 0) {
        node = head->next;
        int roll = rand() % size--;
        for (int i = 0; i < roll; ++i)
            node = node->next;
        list_move_tail(end->prev, node);
        list_move_tail(node, end);
        end = end->prev;
    }
}
