/*
  A synthetic benchmark for measuring data access performance.

  Number of processes to run this program should be N^2, upto 10,000
  (N = 100).
*/
#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>
#include "multi_process_io.h"

static void
add_initial_data(long *data, common_t *common)
{
    for (int i = 0; i < common->val_count; i++) {
	data[i] = common->rank;
    }
}

static void
increase_in_memory_value(long *din, long *dout, common_t *common,
			 MPI_Comm comm)
{
    for (int i = 0; i < common->val_count; i++) {
	dout[i] = din[i] + 1;
    }
#ifdef DEBUG
    MPI_Barrier(comm);
    char comm_name[MPI_MAX_OBJECT_NAME];
    int name_len;
    MPI_Comm_get_name(comm, comm_name, &name_len);
    fprintf(stderr, "Rank[%d]: process key[%s]-val[%ld]\n",
    	    common->rank, comm_name, din[0]);
#endif
}


int
main(int argc, char **argv)
{
    int thlv;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &thlv);
    int nprocs, rank, task_nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    check_nprocs(nprocs, rank, &task_nprocs);

    MPI_Comm even_comm;
    MPI_Comm odd_comm;
    MPI_Comm_split(MPI_COMM_WORLD, rank / task_nprocs + 1, rank, &even_comm);
    MPI_Comm_split(MPI_COMM_WORLD, rank % task_nprocs + 1, rank, &odd_comm);

#ifdef DEBUG
    char even_key[MPI_MAX_OBJECT_NAME];
    snprintf(even_key, MPI_MAX_OBJECT_NAME,
	     "even%06d", (rank / task_nprocs + 1));
    MPI_Comm_set_name(even_comm, even_key);
    char odd_key[MPI_MAX_OBJECT_NAME];
    snprintf(odd_key,  MPI_MAX_OBJECT_NAME,
	     "odd%06d",  (rank % task_nprocs + 1));
    MPI_Comm_set_name(odd_comm, odd_key);
#endif

    common_t common0;
    common0.val_count = VAL_COUNT;
    parse_param(argc, argv, &(common0.val_count));
    common0.rank = rank;
    long *data0 = (long *)malloc(sizeof(long) * common0.val_count);
    add_initial_data(data0, &common0);

    double itr_times[ITERATIONS];
    for (int i = 0; i < ITERATIONS; i++) {
	struct timeval ts;
	measure_time(&ts);
	MPI_Comm comm = (i % 2 == 0) ? even_comm : odd_comm;
	long *data1 = (long *)malloc(sizeof(long) * common0.val_count);
	increase_in_memory_value(data0, data1, &common0, comm);
	free(data0);
	struct timeval te;
	measure_time(&te);
	itr_times[i] = calc_time_diff(&ts, &te);

	data0 = data1;
    }
    free(data0);

    print_time(itr_times, ITERATIONS, rank);

    MPI_Comm_free(&even_comm);
    MPI_Comm_free(&odd_comm);
    MPI_Finalize();
    return 0;
}
