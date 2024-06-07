/*
 * stack.h
 *
 *  Created on: 2023-12-29
 *      Author: xdu
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdbool.h>

#define STATIC 1
#define DYNAMIC 2

#define INIT_MEASURE STATIC

typedef int ST_data_type;

typedef struct stack_data {
	ST_data_type val;

#if INIT_MEASURE == DYNAMIC
	struct stack_data *pre;
#endif
}st_data;

struct stack {
//	st_data *data;
	st_data *top;
	unsigned int capacity;
};

int stack_init(struct stack *st);

int stack_push(struct stack *st, ST_data_type dat);

int stack_pop(struct stack *st, ST_data_type *to);

void stack_clear(struct stack *st);

void destroy_stack(struct stack *st);

#endif /* STACK_H_ */
