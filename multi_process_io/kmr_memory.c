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

static int
add_initial_data(const struct kmr_kv_box kv,
		 const KMR_KVS *kvi, KMR_KVS *kvo, void *p, long i_)
{
    keyval_t *keyval = (keyval_t *)p;
    long *val = (long *)malloc(sizeof(long) * keyval->val_count);
    for (int i = 0; i < keyval->val_count; i++) {
	val[i] = keyval->rank;
    }
    struct kmr_kv_box nkv = { .klen = sizeof(char) * (strlen(keyval->key) + 1),
			      .k.p = keyval->key,
			      .vlen = sizeof(long) * keyval->val_count,
			      .v.p = (void *)val };
    kmr_add_kv(kvo, nkv);
    return MPI_SUCCESS;
}

static int
increment_in_memory_value(const struct kmr_kv_box kv,
			  const KMR_KVS *kvi, KMR_KVS *kvo, void *p, long i_)
{
    keyval_t *keyval = (keyval_t *)p;
    long *src = (long *)kv.v.p;
    long *val = (long *)malloc(kv.vlen);
    for (int i = 0; i < keyval->val_count; i++) {
	val[i] = src[i] + 1;
    }
    struct kmr_kv_box nkv = { .klen = sizeof(char) * (strlen(keyval->key) + 1),
			      .k.p = keyval->key,
			      .vlen = kv.vlen,
			      .v.p = (void *)val };
    kmr_add_kv(kvo, nkv);
#ifdef DEBUG
    fprintf(stderr, "Rank[%d]: process key[%s]-val[%ld]\n",
	    keyval->rank, (char *)kv.k.p, src[0]);
#endif
    return MPI_SUCCESS;
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

    print_time(itr_times, ITERATIONS, rank);

    kmr_free_context(mr);
    kmr_fin();
    MPI_Finalize();
    return 0;
}
