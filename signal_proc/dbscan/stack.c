/**
 * @File: stack.c
 * 
 * @Author: 胡建河
 * @Created on: 2024-01-02
 *
 */

#include "stack.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#if INIT_MEASURE == STATIC

#define MAX_NUM (4096)
#define STACK_NUM (8)
static unsigned char buffer_map = 0;

/* 申请了STACK_NUM个栈，每个栈最多有STACK_NUM元素 */
static st_data data_buffer[STACK_NUM][MAX_NUM];

int init(struct stack *st)
{
	int i;
	int length = sizeof(buffer_map) * 8;

	/* 寻找未被使用的栈，buffer_map的第i位是0，则表示第i个栈尚未被使用 */
	for (i = 0; i < length; ++i) {
		if (!(buffer_map & (1 << i)))
			break;
	}

	if (i == length) {
		printf("init: stack buffer is full.\n");
		return -1;
	}

	/* buffer_map的第i位置1，占用第i个栈 */
	buffer_map |= (1 << i);
	st->top = data_buffer[i];
	st->capacity = 0;

	return 0;
}

int push(struct stack *st, ST_data_type dat)
{
	if (st->capacity >= MAX_NUM) {
		printf("push: stack buffer is full.\n");
		return -1;
	}

	st->top[st->capacity].val = dat;
	++st->capacity;

	return 0;
}

int pop(struct stack *st, ST_data_type *to)
{
	if (st->capacity <= 0) {
		printf("pop: stack buffer is empty.\n");
		return -1;
	}

	--st->capacity;
	*to = st->top[st->capacity].val;

	return 0;
}

void clear(struct stack *st)
{
	st->capacity = 0;
}

void destroy(struct stack *st)
{
	int i;
	int length = sizeof(buffer_map) * 8;

	for (i = 0; i < length; ++i) {
		if (st->top == data_buffer[i]) {
			buffer_map &= ~(1 << i);
		}
	}

	st->top = NULL;
	st->capacity = 0;
}
#endif

#if INIT_MEASURE == DYNAMIC
int init(stack *st)
{
	st->top = (st_data *)malloc(sizeof(st_data));
	if (st->top) {
		printf("init: malloc stack failed.\n");
		return -1;
	}

	st->top->pre = NULL;
	st->capacity = 0;

	return 0;
}

int push(struct stack *st, ST_data_type dat)
{
	st_data *pre;

	st->top->val = dat;
	++st->capacity;

	pre = st->top;
	st->top = (st_data *)malloc(sizeof(st_data));
	if (st->top) {
		printf("push: malloc stack failed.\n");
		return -1;
	}
	st->top->pre = pre;

	return 0;
}

int pop(struct stack *st, ST_data_type *to)
{
	st_data *pre;

	if (st->capacity == 0)
		return -1;

	pre = st->top->pre;
	*to = pre->val;

	free(st->top);
	st->top = pre;

	--st->capacity;

	return 0;
}

void clear(struct stack *st)
{
	st_data *p;

	while (st->top->pre) {
		p = st->top->pre;

		free(st->top);

		st->top = p;
	}

	st->capacity = 0;
}

void destroy(struct stack *st)
{
	st_data *p;

	while (st->top) {
		p = st->top->pre;

		free(st->top);

		st->top = p;
	}

	st->capacity = 0;
}
#endif

int stack_init(struct stack *st)
{
	return init(st);
}

int stack_push(struct stack *st, ST_data_type dat)
{
	return push(st, dat);
}

int stack_pop(struct stack *st, ST_data_type *to)
{
	return pop(st, to);
}

void stack_clear(struct stack *st)
{
	clear(st);
}

void destroy_stack(struct stack *st)
{
	destroy(st);
}
