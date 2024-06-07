/*
 * deque.h
 *
 *  Created on: 2024年6月5日
 *      Author: xdu903
 */

#ifndef _DEQUE_H_
#define _DEQUE_H_

#include <stdbool.h>

#define STATIC_DEQUE_MALLOC 1
#define DYNAMIC_DEQUE_MALLOC 2

#define INIT_DEQUE_MEASURE STATIC_DEQUE_MALLOC

typedef int deque_element_type;

#if INIT_DEQUE_MEASURE == DYNAMIC_DEQUE_MALLOC
typedef struct deque_data {
    deque_element_type val;
    struct deque_data *pre;
    struct deque_data *next;
}qdata;
#endif

#if INIT_DEQUE_MEASURE == STATIC_DEQUE_MALLOC
typedef deque_element_type qdata;
#endif

struct deque {
    int capacity;    /* 队列中元素的数量 */

#if INIT_DEQUE_MEASURE == DYNAMIC_DEQUE_MALLOC
    qdata *front;    /* 头指针 */
    qdata *tail;     /* 尾指针 */
#endif

#if INIT_DEQUE_MEASURE == STATIC_DEQUE_MALLOC
    qdata *data;
    int front;       /* 头指针 */
    int tail;        /* 尾指针 */
#endif
};

int deque_init(struct deque *q);
int deque_push_back(struct deque *q, qdata d);
int deque_pop_front(struct deque *q, qdata *to);
int deque_front(struct deque *q, qdata *to);
void deque_clear(struct deque *q);

static inline bool deque_empty(struct deque *q)
{
    if (!q)
        return false;

    return q->capacity == 0;
}

static inline int deque_size(struct deque *q)
{
    if (!q)
        return 0;

    return q->capacity;
}

void deque_destroy(struct deque *q);

#endif /* _DEQUE_H_ */
