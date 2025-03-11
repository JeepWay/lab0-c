#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q)
        return NULL;

    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe;
    /* cppcheck-suppress unusedLabel */
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);

    free(head);
}

/* Create a element_t object with the given string */
element_t *q_new_element(char *s)
{
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return NULL;

    char *tmp = strdup(s);
    if (!tmp) {
        free(e);
        return NULL;
    }
    e->value = tmp;
    return e;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *e = q_new_element(s);
    if (!e)
        return false;

    list_add(&e->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *e = q_new_element(s);
    if (!e)
        return false;

    list_add_tail(&e->list, head);
    return true;
}

/* Remove an element from head or tail of queue according to  */
element_t *q_remove(struct list_head *head,
                    bool from_head,
                    char *sp,
                    size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *element;
    if (from_head)
        element = list_first_entry(head, element_t, list);
    else
        element = list_last_entry(head, element_t, list);
    list_del_init(&element->list);

    if (sp) {
        size_t len = strlen(element->value) + 1;
        memcpy(sp, element->value, len);
    }
    return element;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head, true, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head, false, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int size = 0;
    struct list_head *node;

    list_for_each (node, head)
        size++;

    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *left = head->next;
    struct list_head *right = head->prev;

    while (left != right && left->next != right) {
        left = left->next;
        right = right->prev;
    }
    // right is the middle node
    element_t *element = list_entry(right, element_t, list);
    list_del_init(right);
    q_release_element(element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    // use list_head `removed` to collect the nodes to be removed
    LIST_HEAD(removed);
    element_t *node, *safe;
    bool has_duplicate = false;

    /* cppcheck-suppress unusedLabel */
    list_for_each_entry_safe (node, safe, head, list) {
        if (&safe->list != head && !strcmp(node->value, safe->value)) {
            list_move_tail(&node->list, &removed);
            has_duplicate = true;
        } else if (has_duplicate) {
            list_move_tail(&node->list, &removed);
            has_duplicate = false;
        }
    }

    // free the memory used by duplicated nodes in removed linked list
    /* cppcheck-suppress unusedLabel */
    list_for_each_entry_safe (node, safe, &removed, list)
        q_release_element(node);

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    return q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;

    LIST_HEAD(sub_list);
    int count = 0;
    struct list_head *cur_head = head;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        count++;
        list_move(node, &sub_list);
        if (count == k) {
            list_splice_init(&sub_list, cur_head);
            cur_head = safe->prev;
            count = 0;
        }
    }
    q_reverse(&sub_list);
    list_splice_init(&sub_list, cur_head);
}

static void q_merge_two(struct list_head *head,
                        struct list_head *left,
                        struct list_head *right,
                        bool descend)
{
    struct list_head *l = left->next;
    struct list_head *r = right->next;

    while (l != left && r != right) {
        const element_t *l_elem = list_entry(l, element_t, list);
        const element_t *r_elem = list_entry(r, element_t, list);

        int cmp = strcmp(l_elem->value, r_elem->value);
        if (descend ? (cmp >= 0) : (cmp <= 0)) {
            l = l->next;
            list_move_tail(l->prev, head);
        } else {
            r = r->next;
            list_move_tail(r->prev, head);
        }
    }
    /* Append remaining nodes */
    if (l != left)
        list_splice_tail_init(left, head);
    if (r != right)
        list_splice_tail_init(right, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *front = head->next;
    struct list_head *end = head->prev;
    /* Find the middle node of the linked list */
    while (front != end && front->next != end) {
        front = front->next;
        end = end->prev;
    }
    struct list_head *mid = end;

    LIST_HEAD(left);
    LIST_HEAD(right);
    list_splice_tail_init(head, &right);
    list_cut_position(&left, &right, mid->prev);
    q_sort(&left, descend);
    q_sort(&right, descend);
    q_merge_two(head, &left, &right, descend);
}

/* Remove every node which has a node with a strictly less/greater value
 * anywhere to the right side of it according to descend flag */
int q_monotonic(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;

    element_t *node, *safe;
    node = descend ? list_last_entry(head, element_t, list)
                   : list_first_entry(head, element_t, list);
    safe = list_entry(descend ? node->list.prev : node->list.next, element_t,
                      list);
    /* cppcheck-suppress constVariablePointer */
    char *max = node->value;

    while (&node->list != head) {
        if (strcmp(max, node->value) > 0) {
            list_del_init(&node->list);
            q_release_element(node);
        } else
            max = node->value;

        node = safe;
        safe = list_entry(descend ? node->list.prev : node->list.next,
                          element_t, list);
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_monotonic(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_monotonic(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head) || list_is_singular(head))
        return q_size(head);

    queue_contex_t *current = list_entry(head->next, queue_contex_t, chain);
    queue_contex_t *next = list_entry(head->next->next, queue_contex_t, chain);
    int queue_num = q_size(head);
    while (queue_num > 1) {
        LIST_HEAD(merged);
        q_merge_two(&merged, current->q, next->q, descend);
        list_splice_tail_init(&merged, current->q);
        next = list_entry(next->chain.next, queue_contex_t, chain);
        queue_num--;
    }
    return q_size(current->q);
}
