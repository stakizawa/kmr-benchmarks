#ifndef MULTI_PROCESS_IO_H
#define MULTI_PROCESS_IO_H

#include <stdlib.h>

#define KEY_LEN 12
#define VAL_COUNT 1000
#define ITERATIONS 10

void
check_nprocs(int nprocs, int rank, int *task_nprocs)
{
    int n = 101;
    int allowed[n];
    for (int i = 0; i < n; i++) {
	allowed[i] = i * i;
    }
    _Bool included = 0;
    for (int i = 0; i < n; i++) {
	if (nprocs == allowed[i]) {
	    included = 1;
	    *task_nprocs = i;
	    break;
	}
    }
    if (!included) {
	if (rank == 0) {
	    fprintf(stderr, "Number of processes should be N^2.\n");
	}
	MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

void
parse_param(int argc, char **argv, int *val_count)
{
    if (argc == 2) {
	*val_count = atoi(argv[1]);
    }
}

static int
measure_time(struct timeval *tv)
{
    MPI_Barrier(MPI_COMM_WORLD);
    if (gettimeofday(tv, NULL) == -1) {
        perror("gettimeofday");
        return -1;
    }
    return 0;
}

static double
calc_time_diff(struct timeval *tv_s, struct timeval *tv_e)
{
    return ((double)tv_e->tv_sec - (double)tv_s->tv_sec)
        + ((double)tv_e->tv_usec - (double)tv_s->tv_usec) /1000000.0;
}

typedef struct {
    char *key;
    int val_count;
    int rank;
} keyval_t;

#endif
