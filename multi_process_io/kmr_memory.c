/*
  A synthetic benchmark for measuring data access performance.

  Number of processes to run this program should be N^2, upto 10,000
  (N = 100).
*/
#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>
#include <kmr.h>
#include "multi_process_io.h"


int
main(int argc, char **argv)
{
    int thlv;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &thlv);
    int nprocs, rank, task_nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    check_nprocs(nprocs, rank, &task_nprocs);
    kmr_init();
    KMR *mr = kmr_create_context(MPI_COMM_WORLD, MPI_INFO_NULL, 0);

    char even_key[KEY_LEN];
    char odd_key[KEY_LEN];
    snprintf(even_key, KEY_LEN, "even%06d", (rank / task_nprocs + 1));
    snprintf(odd_key,  KEY_LEN, "odd%06d",  (rank % task_nprocs + 1));

    keyval_t keyval0;
    keyval0.key = even_key;
    keyval0.val_count = VAL_COUNT;
    parse_param(argc, argv, &(keyval0.val_count));
    keyval0.rank = rank;
    KMR_KVS *kvs0 = kmr_create_kvs(mr, KMR_KV_OPAQUE, KMR_KV_OPAQUE);
    kmr_map_once(kvs0, &keyval0, kmr_noopt, 0, add_initial_data);

    double itr_times[ITERATIONS];
    for (int i = 0; i < ITERATIONS; i++) {
	keyval_t keyval1;
	keyval1.key = (i % 2 == 0)? odd_key : even_key;
	keyval1.val_count = keyval0.val_count;
	keyval1.rank = rank;
	KMR_KVS *kvs1 = kmr_create_kvs(mr, KMR_KV_OPAQUE, KMR_KV_OPAQUE);

	struct timeval ts;
	measure_time(&ts);
	kmr_map_multiprocess_by_key(kvs0, kvs1, &keyval1, kmr_noopt, rank,
				    increment_in_memory_value);
	struct timeval te;
	measure_time(&te);
	itr_times[i] = calc_time_diff(&ts, &te);

	kvs0 = kvs1;
    }
    kmr_free_kvs(kvs0);

    if (rank == 0) {
	double time_sum = 0.0;
	for (int i = 0; i < ITERATIONS; i++) {
	    time_sum += itr_times[i];
	    printf("Iteration[%02d]  %f\n", i, itr_times[i]);
	}
	printf("Total  %f\n", time_sum);
    }

    kmr_free_context(mr);
    kmr_fin();
    MPI_Finalize();
    return 0;
}
