/*
 * dbscan.h
 *
 *  Created on: 2024-2-27
 *      Author: xdu
 */

#ifndef DBSCAN_H_
#define DBSCAN_H_

#include "stack.h"
#include "deque.h"
#include <stdbool.h>
#include "srio_adapter.h"

typedef struct pdw {
	/*
	 * data format:
	 *     bit 0~19 represent decimal;
	 *     bit 20~31 represent integer.
	 */
	unsigned int aoa : 32;		/* angle of arrival */
	unsigned int freq : 32;		/* carrier frequency */
	unsigned int pw : 32;		/* pulse width */
}pdw_st;

typedef struct dbscan {
	pdw_st *set;
	int *major;		/* dbsacn聚类后，point_set中各项对应的类的编号  */
	int ngroup;
	unsigned int capacity;		/* point_set中数据的总数  */
	int *visited;

#define UNLABELED 0
#define LABELED   1
#define CENTER    2
#define EDGE      3
#define NOISE     4

//	struct stack new_nbr;
	struct deque finded_pts;
}dbscan_st;

int init_dbscan(dbscan_st *db, unsigned int num);

int get_data(dbscan_st *db, const ORIG_PDW *src);

void dbscan(dbscan_st *db, unsigned int e, unsigned int minpts);

void del_dbscan(dbscan_st *db);

void print_dbscan_result(dbscan_st *db);

#endif /* DBSCAN_H_ */
