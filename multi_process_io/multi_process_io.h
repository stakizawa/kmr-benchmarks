#ifndef MULTI_PROCESS_IO_H
#define MULTI_PROCESS_IO_H

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define KEY_LEN 12
#define FILENAME_LEN 64
#define VAL_COUNT     1048576  /* 8MB in long */
#define MAX_VAL_COUNT 4194304  /* 32MB in long */
#define FILE_SIZE     64       /* 64MB */
#define MAX_FILE_SIZE 1024     /* 1GB */
#define IO_COUNT (1024 * 1024 / sizeof(long))
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
    *val_count = VAL_COUNT;
    if (argc == 2) {
	*val_count = atoi(argv[1]);
    }
    if (*val_count > MAX_VAL_COUNT) {
	fprintf(stderr, "value count should be less than %d.\n",
		MAX_VAL_COUNT);
	exit(1);
    }
}

void
parse_param_file(int argc, char **argv, size_t *file_size)
{
    *file_size = FILE_SIZE;
    if (argc == 2) {
	*file_size = atol(argv[1]);
    }
    if (*file_size > MAX_FILE_SIZE) {
	fprintf(stderr, "file size should be less than %d.\n",
		MAX_FILE_SIZE);
	exit(1);
    }
}

int
measure_time(struct timeval *tv)
{
    MPI_Barrier(MPI_COMM_WORLD);
    if (gettimeofday(tv, NULL) == -1) {
        perror("gettimeofday");
        return -1;
    }
    return 0;
}

double
calc_time_diff(struct timeval *tv_s, struct timeval *tv_e)
{
    return ((double)tv_e->tv_sec - (double)tv_s->tv_sec)
        + ((double)tv_e->tv_usec - (double)tv_s->tv_usec) /1000000.0;
}

void
print_time(double *times, int time_count, int rank)
{
    if (rank == 0) {
	double time_sum = 0.0;
	for (int i = 0; i < time_count; i++) {
	    time_sum += times[i];
	    printf("Iteration[%02d]  %f\n", i, times[i]);
	}
	printf("Total  %f\n", time_sum);
    }
}

void
create_file(int rank, int iteration, size_t size,
	    char *filename, size_t filename_len)
{
    snprintf(filename, filename_len, "./%06d-%02d.dat", rank, iteration);
    /* write 1MB at once */
    long *buf = (long *)malloc(sizeof(long) * IO_COUNT);
    for (int i = 0; i < IO_COUNT; i++) {
	buf[i] = rank;
    }

    FILE *fp = fopen(filename, "w+");
    assert(fp != 0);
    for (int i = 0; i < size; i++) {
	size_t cc = fwrite(buf, sizeof(long), IO_COUNT, fp);
	assert(cc == IO_COUNT);
    }
    fclose(fp);
}

void
delete_file(int rank, int iteration)
{
    char filename[FILENAME_LEN];
    snprintf(filename, FILENAME_LEN, "./%06d-%02d.dat", rank, iteration);
    unlink(filename);
}

typedef struct {
    char *key;
    int val_count;
    int rank;
    size_t file_size;
    int iteration;
} common_t;

#endif
