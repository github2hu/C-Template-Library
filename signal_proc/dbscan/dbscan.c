/*
 * dbscan.c
 *
 *  Created on: 2024-2-27
 *      Author: xdu
 */

#include "dbscan.h"
#include <stdlib.h>
#include <c6x.h>

#define MAX_NUM (4096)

#define STATIC 1
#define DYNAMIC 2

#define INIT_MEASURE STATIC

#define RUNTIME_DEBUG 1

#define _DEBUG 0

#if _DEBUG
#define debug(fmt, ...) \
printf("Debug info: %s function %d line " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if INIT_MEASURE == STATIC
#pragma DATA_SECTION(major, ".static_var")
int major[MAX_NUM];

#pragma DATA_SECTION(visited, ".static_var")
int visited[MAX_NUM];

#pragma DATA_SECTION(set, ".static_var")
pdw_st set[MAX_NUM];

#pragma DATA_SECTION(new_nbrs, ".static_var")
int new_nbrs[MAX_NUM];

#if RUNTIME_DEBUG
    unsigned long long stime, etime;
#endif

static int init(dbscan_st *db, unsigned int num)
{
	db->set = set;
	db->major = major;
	db->visited = visited;

	/* 初始化栈 */
//	stack_init(&db->new_nbr);
	deque_init(&db->finded_pts);

	memset(db->major, -1, sizeof(db->major[0]) * MAX_NUM);
	memset(db->visited, UNLABELED, sizeof(db->visited[0]) * MAX_NUM);

	return 0;
}

static void del(dbscan_st *db)
{
	db->set = NULL;
	db->major = NULL;
	db->visited = NULL;

	deque_destroy(&db->finded_pts);
//	destroy_stack(&db->new_nbr);
}
#endif

#if INIT_MEASURE == DYNAMIC
static int init(dbscan_st *db, unsigned int num)
{
	int i;

	/* 为数据集分配空间 */
	db->set = (pdw_st *)malloc(sizeof(pdw_st) * num);
	if (!db->set) {
#if _DEBUG
		debug("malloc mem to set failure");
#endif
		goto error1;
	}

	db->major = (int *)malloc(sizeof(int) * num);
	if (!db->major) {
#if _DEBUG
		debug("malloc mem to major failure");
#endif

		goto error2;
	}

	db->visited = (bool *)malloc(sizeof(bool) * num);
	if (!db->visited) {
#if _DEBUG
		debug("malloc mem to visited failure");
#endif

		goto error3;
	}

	/* 初始化栈 */
	stack_init(&db->new_nbr);
	stack_init(&db->finded_nbr);

	for (i = 0; i < num; ++i) {
		db->major[i] = -1;
		db->visited[i] = false;
	}

	return 0;

error3:
	free(db->major);
	db->major = NULL;

error2:
	free(db->set);
	db->set = NULL;

error1:
		return -1;
}

static void del(dbscan_st *db)
{
	/* 如果db->set初始化过，则释放 */
	if (db->set) {
		free(db->set);
		db->set = NULL;
	}

	if (db->major) {
		free(db->major);
		db->major = NULL;
	}

	if (db->visited) {
		free(db->visited);
		db->visited = NULL;
	}

	destroy_stack(&db->finded_nbr);
	destroy_stack(&db->new_nbr);
}
#endif

int init_dbscan(dbscan_st *db, unsigned int num)
{
	db->capacity = num;

	return init(db, num);
}

void del_dbscan(dbscan_st *db)
{
	del(db);
}

int get_data(dbscan_st *db, const ORIG_PDW *src)
{
	int i = 0;

	while (i < db->capacity) {
		pdw_st *p = &(db->set[i]);

		p->aoa = src[i].AOA;
		p->freq = src[i].FC;
		p->pw = src[i].PW;

		++i;
	}

	return i;
}

static inline unsigned int distance(pdw_st *p1, pdw_st *p2)
{
	/* T = |A(:, k) ^ W(:, j)| / (|W(:, j)| + a) */

	return abs(p1->aoa - p2->aoa) + abs(p1->pw - p2->pw);
}

static int search_nbr(dbscan_st *db, int point, unsigned int e)
{
	int j = 0;
	int nnbr = 0;
	int length = db->capacity;
	pdw_st *pdw_set = db->set;
	pdw_st *src_point = &(pdw_set[point]);

	for (j = 0; j < length; ++j) {
        if (distance(src_point, &(pdw_set[j])) > e)
		    continue;

        new_nbrs[nnbr] = j;
		++nnbr;
	}

	return nnbr;
}

void dbscan(dbscan_st *db, unsigned int e, unsigned int minpts)
{
    int i = 0, j, k;
    int g = 0;
    int nnbr;

    for (i = 0; i < db->capacity; ++i) {
        /* 若i的状态为LABELED，表示i已经被标记过 */
        if (db->visited[i] == LABELED)
            continue;
#if RUNTIME_DEBUG
    TSCH = 0;
    TSCL = 0;
    stime = _itoll(TSCH, TSCL);
#endif

        /* 寻找i的e领域内的所有点 */
        nnbr = search_nbr(db, i, e);

#if RUNTIME_DEBUG
    etime = _itoll(TSCH, TSCL);
    printf("search_nbr's running cycle is %llu.\n", etime - stime);
#endif

        /* 若i不是核心点，则标记为边界点，继续寻找核心点 */
        if (nnbr < minpts) {
            db->visited[i] = EDGE;
            continue;
        }

        ++g;
        db->visited[i] = LABELED;
        db->major[i] = g;

        for (k = 0; k < nnbr; ++k) {
            j = new_nbrs[k];

            if (db->visited[j] == LABELED)
                continue;

            /* 若j不是边界点，则它可能有密度直达点  */
            if (db->visited[j] != EDGE) {
            #if RUNTIME_DEBUG
                TSCH = 0;
                TSCL = 0;
                stime = _itoll(TSCH, TSCL);
            #endif

                deque_push_back(&db->finded_pts, j);

            #if RUNTIME_DEBUG
                etime = _itoll(TSCH, TSCL);
                printf("deque_push_back's running cycle is %llu.\n", etime - stime);
            #endif
            }

            db->visited[j] = LABELED;
            db->major[j] = g;
        }

        /* 寻找i密度可达的点 */
        while (!deque_empty(&db->finded_pts)) {
        #if RUNTIME_DEBUG
            TSCH = 0;
            TSCL = 0;
            stime = _itoll(TSCH, TSCL);
        #endif

            deque_pop_front(&db->finded_pts, &j);

        #if RUNTIME_DEBUG
            etime = _itoll(TSCH, TSCL);
            printf("deque_pop_front's running cycle is %llu.\n", etime - stime);
        #endif

            /* j是i密度直达或密度可达的点, 寻找j的e领域内的所有的点 */
            nnbr = search_nbr(db, j, e);

            /* j不是核心点，j的e领域内的点不是j的密度直达点，也就不是i的密度可达点 */
            if (nnbr < minpts) {
                continue;
            }

            /* j是核心点，那么j的密度直达点就是i的密度可达点 */
            for (k = 0; k < nnbr; ++k) {
                j = new_nbrs[k];

                if (db->visited[j] == LABELED)
                    continue;

                /* 若j不是边界点，则它可能有密度直达点  */
                if (db->visited[j] != EDGE)
                    deque_push_back(&db->finded_pts, j);

                db->visited[j] = LABELED;
                db->major[j] = g;
            }
        }
    }

    db->ngroup = g;
}

void print_dbscan_result(dbscan_st *db)
{
	FILE *fptr = NULL;
	int i;

	fptr = fopen("../simulate/dbscan_sim.txt", "w+");
	if (!fptr) {
		printf("can't open sim.txt");
		return;
	}

	fprintf(fptr, "---------------------------------------------------------------------------------------------\n");
	fprintf(fptr, "num                   PW                   FC                   AOA                   cluster\n");
	fprintf(fptr, "---------------------------------------------------------------------------------------------\n");
	for (i = 0; i < db->capacity; ++i) {
		fprintf(fptr, "%-21d %-20d %-20d %-21d %-7d\n", \
				i, db->set[i].pw, db->set[i].freq, db->set[i].aoa, db->major[i]);
	}

	fprintf(fptr, "---------------------------------------------------------------------------------------------\n\n");

	fclose(fptr);
}/*
 * dbscan.c
 *
 *  Created on: 2024-2-27
 *      Author: xdu
 */

#include "dbscan.h"
#include <stdlib.h>

#define MAX_NUM (4096)

#define STATIC 1
#define DYNAMIC 2

#define INIT_MEASURE STATIC

#define _DEBUG 0

#if _DEBUG
#define debug(fmt, ...) \
printf("Debug info: %s function %d line " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if INIT_MEASURE == STATIC
#pragma DATA_SECTION(major, ".static_var")
int major[MAX_NUM];

#pragma DATA_SECTION(visited, ".static_var")
bool visited[MAX_NUM];

#pragma DATA_SECTION(set, ".static_var")
pdw_st set[MAX_NUM];

static int init(dbscan_st *db, unsigned int num)
{
	int i;
	db->set = set;
	db->major = major;
	db->visited = visited;

	/* 初始化栈 */
	stack_init(&db->new_nbr);
	stack_init(&db->finded_nbr);

	for (i = 0; i < MAX_NUM; ++i) {
		db->major[i] = -1;
		db->visited[i] = false;
	}

	return 0;
}

static void del(dbscan_st *db)
{
	db->set = NULL;
	db->major = NULL;
	db->visited = NULL;

	destroy_stack(&db->finded_nbr);
	destroy_stack(&db->new_nbr);
}
#endif

#if INIT_MEASURE == DYNAMIC
static int init(dbscan_st *db, unsigned int num)
{
	int i;

	/* 为数据集分配空间 */
	db->set = (pdw_st *)malloc(sizeof(pdw_st) * num);
	if (!db->set) {
#if _DEBUG
		debug("malloc mem to set failure");
#endif
		goto error1;
	}

	db->major = (int *)malloc(sizeof(int) * num);
	if (!db->major) {
#if _DEBUG
		debug("malloc mem to major failure");
#endif

		goto error2;
	}

	db->visited = (bool *)malloc(sizeof(bool) * num);
	if (!db->visited) {
#if _DEBUG
		debug("malloc mem to visited failure");
#endif

		goto error3;
	}

	/* 初始化栈 */
	stack_init(&db->new_nbr);
	stack_init(&db->finded_nbr);

	for (i = 0; i < num; ++i) {
		db->major[i] = -1;
		db->visited[i] = false;
	}

	return 0;

error3:
	free(db->major);
	db->major = NULL;

error2:
	free(db->set);
	db->set = NULL;

error1:
		return -1;
}

static void del(dbscan_st *db)
{
	/* 如果db->set初始化过，则释放 */
	if (db->set) {
		free(db->set);
		db->set = NULL;
	}

	if (db->major) {
		free(db->major);
		db->major = NULL;
	}

	if (db->visited) {
		free(db->visited);
		db->visited = NULL;
	}

	destroy_stack(&db->finded_nbr);
	destroy_stack(&db->new_nbr);
}
#endif

int init_dbscan(dbscan_st *db, unsigned int num)
{
	db->capacity = num;

	return init(db, num);
}

void del_dbscan(dbscan_st *db)
{
	del(db);
}

int get_data(dbscan_st *db, const ORIG_PDW *src)
{
	int i = 0;

	while (i < db->capacity) {
		pdw_st *p = &(db->set[i]);

		p->aoa = src[i].AOA;
		p->freq = src[i].FC;
		p->pw = src[i].PW;

		++i;
	}

	return i;
}

static inline unsigned int distance(pdw_st p1, pdw_st p2)
{
	/* T = |A(:, k) ^ W(:, j)| / (|W(:, j)| + a) */
	unsigned int ret = 0;

	ret = abs(p1.aoa - p2.aoa) + abs(p1.freq - p2.freq) + abs(p1.pw - p2.pw) / 70;

	return ret;
}

static void search_enbr(dbscan_st *db, int point, unsigned int e)
{
	int j = 0;
	pdw_st src_point = db->set[point];

	for (j = 0; j < db->capacity; ++j) {
		if (j == point)
			continue;

		if (distance(src_point, db->set[j]) < e) {
			stack_push(&db->new_nbr, j);
		}
	}
}

void dbscan(dbscan_st *db, unsigned int e, unsigned int minpts)
{
	int i = 0;
	int j = 0;
	int group = 0;

	for (i = 0; i < db->capacity; ++i) {
		if (db->visited[i] == LABELED)
			continue;

		search_enbr(db, i, e);

		/* i不是核心点 */
		if (db->new_nbr.capacity < minpts - 1) {
			stack_clear(&db->new_nbr);
//			stack_clear(&db->finded_nbr);
			continue;
		}

		db->visited[i] = LABELED;
		db->major[i] = group;

		/* 对新增的点进行标记 */
		while (db->new_nbr.capacity != 0) {
			stack_pop(&db->new_nbr, &j);

			if (db->visited[j] == LABELED)
				continue;

			stack_push(&db->finded_nbr, j);
			db->visited[j] = LABELED;
			db->major[j] = db->major[i];
		}

		/* 如果邻域内所有的点都已被搜索过，则退出 */
		while (db->finded_nbr.capacity != 0) {
			stack_pop(&db->finded_nbr, &j);

			search_enbr(db, j, e);

			if (db->new_nbr.capacity < minpts - 1) {
				stack_clear(&db->new_nbr);
//				stack_clear(&db->finded_nbr);
				continue;
			}

			/* 对新增的点进行标记 */
			while (db->new_nbr.capacity != 0) {
				stack_pop(&db->new_nbr, &j);

				if (db->visited[j] == LABELED)
					continue;

				stack_push(&db->finded_nbr, j);
				db->visited[j] = LABELED;
				db->major[j] = db->major[i];
			}
		}

		++group;
	}

	db->ngroup = group;
}

void print_dbscan_result(dbscan_st *db)
{
	FILE *fptr = NULL;
	int i;

	fptr = fopen("../simulate/dbscan_sim.txt", "w+");
	if (!fptr) {
		printf("can't open sim.txt");
		return;
	}

	fprintf(fptr, "---------------------------------------------------------------------------------------------\n");
	fprintf(fptr, "num                   PW                   FC                   AOA                   cluster\n");
	fprintf(fptr, "---------------------------------------------------------------------------------------------\n");
	for (i = 0; i < db->capacity; ++i) {
		fprintf(fptr, "%-21d %-20d %-20d %-21d %-7d\n", \
				i, db->set[i].pw, db->set[i].freq, db->set[i].aoa, db->major[i]);
	}

	fprintf(fptr, "---------------------------------------------------------------------------------------------\n\n");

	fclose(fptr);
}
