/*
  A synthetic benchmark for measuring data access performance.

  Number of processes to run this program should be N^2, upto 10,000
  (N = 100).

  This program accepts an argument that represents size of a file each
  process generates in an iteration in MB.  The minimum file size is
  1MB (specify 1) and the maximum file size is 1GB (specify 1024).
  The default is 64MB.

  $ mpiexec -np 64 ./less_locality_file 128
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
    common_t *common = (common_t *)p;
    char filename[FILENAME_LEN];
    create_file(common->rank, common->iteration, common->file_size,
		filename, FILENAME_LEN);
    common->val_count = IO_COUNT * common->file_size;
    struct kmr_kv_box nkv = { .klen = sizeof(char) * (strlen(common->key) + 1),
			      .k.p = common->key,
			      .vlen = sizeof(char) * (strlen(filename) + 1),
			      .v.p = (void *)filename };
    kmr_add_kv(kvo, nkv);
    return MPI_SUCCESS;
}

static int
increment_in_file_value(const struct kmr_kv_box kv,
			const KMR_KVS *kvi, KMR_KVS *kvo, void *p, long i_)
{
    common_t *common = (common_t *)p;
    char *infile = (char *)kv.v.p;
    char outfile[FILENAME_LEN];
    snprintf(outfile, FILENAME_LEN, "./%06d-%02d.dat", common->rank,
	     common->iteration + 1);

    FILE *ifp = fopen(infile, "r");
    FILE *ofp = fopen(outfile, "w+");
    assert(ifp != 0 && ofp != 0);
    /* read/write 1MB at once */
    long *buf = (long *)malloc(sizeof(long) * IO_COUNT);
    for (int i = 0; i < common->file_size; i++) {
	size_t cc = fread(buf, sizeof(long), IO_COUNT, ifp);
	assert(cc == IO_COUNT);
	for (int j = 0; j < IO_COUNT; j++) {
	    buf[j] += 1;
	}
	cc = fwrite(buf, sizeof(long), IO_COUNT, ofp);
	assert(cc == IO_COUNT);
    }
    free(buf);
    fclose(ofp);

    struct kmr_kv_box nkv = { .klen = sizeof(char) * (strlen(common->key) + 1),
			      .k.p = common->key,
			      .vlen = sizeof(char) * (strlen(outfile) + 1),
			      .v.p = (void *)outfile };
    kmr_add_kv(kvo, nkv);
#ifdef DEBUG
    fseek(ifp, 0, SEEK_SET);
    long val;
    fread(&val, sizeof(long), 1, ifp);
    fprintf(stderr, "Rank[%d]: process key[%s]-val[%ld]\n",
	    common->rank, (char *)kv.k.p, val);
#endif
    fclose(ifp);
    delete_file(common->rank, common->iteration);

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

    common_t common0;
    common0.key = even_key;
    parse_param_file(argc, argv, &(common0.file_size));
    common0.rank = rank;
    common0.iteration = 0;
    KMR_KVS *kvs0 = kmr_create_kvs(mr, KMR_KV_OPAQUE, KMR_KV_OPAQUE);
    kmr_map_once(kvs0, &common0, kmr_noopt, 0, add_initial_data);

    double itr_times[ITERATIONS];
    for (int i = 0; i < ITERATIONS; i++) {
	common0.key = (i % 2 == 0)? odd_key : even_key;
	common0.iteration = i;
	KMR_KVS *kvs1 = kmr_create_kvs(mr, KMR_KV_OPAQUE, KMR_KV_OPAQUE);

	struct timeval ts;
	measure_time(&ts);
	kmr_map_multiprocess_by_key(kvs0, kvs1, &common0, kmr_noopt, rank,
				    increment_in_file_value);
	struct timeval te;
	measure_time(&te);
	itr_times[i] = calc_time_diff(&ts, &te);

	kvs0 = kvs1;
    }
    kmr_free_kvs(kvs0);
    delete_file(common0.rank, common0.iteration + 1);

    print_time(itr_times, ITERATIONS, rank);

    kmr_free_context(mr);
    kmr_fin();
    MPI_Finalize();
    return 0;
}
