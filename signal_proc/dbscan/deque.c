/*
 * deque.c
 *
 *  Created on: 2024年6月5日
 *      Author: xdu903
 */

#include "deque.h"
#include <stdio.h>
#include <stdlib.h>

#if INIT_DEQUE_MEASURE == STATIC_DEQUE_MALLOC
#define MAX_NUM (4096)
#define DEQUE_NUM (8)
static unsigned char buffer_map = 0;

/* 申请了DEQUE_NUM个队列，每个队列最多有MAX_NUM元素 */
#pragma DATA_SECTION(qdata_buffer, ".static_var")
qdata qdata_buffer[DEQUE_NUM][MAX_NUM];
#endif

static int init(struct deque *q);
static inline int push_back(struct deque *q, qdata d);
static inline int pop_front(struct deque *q, qdata *to);
static inline int front(struct deque *q, qdata *to);
static inline void clear(struct deque *q);
static void destroy(struct deque *q);

int deque_init(struct deque *q)
{
    if (!q) {
        printf("Queue not exist\n");
        return -1;
    }

    return init(q);
}

int deque_push_back(struct deque *q, qdata d)
{
    if (!q || !q->data) {
        printf("Queue is not initialized\n");
        return -1;
    }

    if (q->capacity >= MAX_NUM) {
        printf("push_back: deque buffer is full.\n");
        return -2;
    }

    return push_back(q, d);
}

int deque_pop_front(struct deque *q, qdata *to)
{
    if (!q || !q->data) {
        printf("Queue is not initialized\n");
        return -1;
    }

    if (q->capacity <= 0) {
        printf("push_back: deque buffer is empty.\n");
        return -2;
    }

    return pop_front(q, to);
}

int deque_front(struct deque *q, qdata *to)
{
    if (!q || !q->data) {
        printf("Queue is not initialized\n");
        return -1;
    }

    if (q->capacity <= 0) {
        printf("push_back: deque buffer is empty.\n");
        return -2;
    }

    return front(q, to);
}

void deque_clear(struct deque *q)
{
    if (!q || !q->data) {
        printf("Queue is not initialized\n");
        return;
    }

    clear(q);
}

void deque_destroy(struct deque *q)
{
    if (!q || !q->data) {
        printf("Queue is not initialized\n");
        return;
    }

    destroy(q);
}

#if INIT_DEQUE_MEASURE == STATIC_DEQUE_MALLOC

static int init(struct deque *q)
{
    int i;
    int length = sizeof(buffer_map) * 8;

    /* 寻找未被使用的队列，buffer_map的第i位是0，则表示第i个队列尚未被使用 */
    for (i = 0; i < length; ++i) {
        if (!(buffer_map & (1 << i)))
            break;
    }

    if (i == length) {
        printf("Init: deque is full.\n");
        return -2;
    }

    /* buffer_map的第i位置1，占用第i个队列 */
    buffer_map |= (1 << i);
    q->data = qdata_buffer[i];
    q->capacity = 0;
    q->front = -1;
    q->tail = -1;

    return 0;
}

static inline int push_back(struct deque *q, qdata d)
{
    ++q->capacity;
    q->tail = (q->tail + 1) % MAX_NUM;
    q->data[q->tail] = d;

    return 0;
}

static inline int pop_front(struct deque *q, qdata *to)
{
    --q->capacity;
    q->front = (q->front + 1) % MAX_NUM;
    *to = q->data[q->front];

    return 0;
}

static inline int front(struct deque *q, qdata *to)
{
    int pos;

    pos = (q->front + 1) % MAX_NUM;
    *to = q->data[pos];

    return 0;
}

static inline void clear(struct deque *q)
{
    q->capacity = 0;
    q->front = q->tail = -1;
}

static void destroy(struct deque *q)
{
    int i;
    int length = sizeof(buffer_map) * 8;

    /* 找到队列使用的buffer i，把buffer_map的第i位置零 */
    for (i = 0; i < length; ++i) {
        if (q->data == qdata_buffer[i]) {
            buffer_map &= ~(1 << i);
        }
    }

    q->data = NULL;
}
#endif
