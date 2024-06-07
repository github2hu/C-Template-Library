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

int major[MAX_NUM];
int visited[MAX_NUM];
pdw_st set[MAX_NUM];
int new_nbrs[MAX_NUM];

static int init(dbscan_st *db, unsigned int num)
{
	db->set = set;
	db->major = major;
	db->visited = visited;

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
}
#endif

#if INIT_MEASURE == DYNAMIC
static int init(dbscan_st *db, unsigned int num)
{
	/* 待实现 */
}

static void del(dbscan_st *db)
{
	/* 待实现 */
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

        /* 寻找i的e领域内的所有点 */
        nnbr = search_nbr(db, i, e);

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
                deque_push_back(&db->finded_pts, j);

            db->visited[j] = LABELED;
            db->major[j] = g;
        }

        /* 寻找i密度可达的点 */
        while (!deque_empty(&db->finded_pts)) {
            deque_pop_front(&db->finded_pts, &j);

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
}