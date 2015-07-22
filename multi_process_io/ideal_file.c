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
add_initial_data(common_t *common)
{
    char filename[FILENAME_LEN];
    create_file(common->rank, common->iteration, common->file_size,
		filename, FILENAME_LEN);
    common->val_count = IO_COUNT * common->file_size;
}

static void
increase_in_file_value(common_t *common, MPI_Comm comm)
{
    char infile[FILENAME_LEN];
    snprintf(infile, FILENAME_LEN, "./%06d-%02d.dat", common->rank,
	     common->iteration);
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
    fclose(ofp);
#ifdef DEBUG
    MPI_Barrier(comm);
    char comm_name[MPI_MAX_OBJECT_NAME];
    int name_len;
    MPI_Comm_get_name(comm, comm_name, &name_len);
    fseek(ifp, 0, SEEK_SET);
    long val;
    fread(&val, sizeof(long), 1, ifp);
    fprintf(stderr, "Rank[%d]: process key[%s]-val[%ld]\n",
    	    common->rank, comm_name, val);
#endif
    fclose(ifp);

    delete_file(common->rank, common->iteration);
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
    parse_param_file(argc, argv, &(common0.file_size));
    common0.rank = rank;
    common0.iteration = 0;
    add_initial_data(&common0);

    double itr_times[ITERATIONS];
    for (int i = 0; i < ITERATIONS; i++) {
	common0.iteration = i;
	struct timeval ts;
	measure_time(&ts);
	MPI_Comm comm = (i % 2 == 0) ? even_comm : odd_comm;
	increase_in_file_value(&common0, comm);
	struct timeval te;
	measure_time(&te);
	itr_times[i] = calc_time_diff(&ts, &te);
    }
    delete_file(common0.rank, common0.iteration + 1);

    print_time(itr_times, ITERATIONS, rank);

    MPI_Comm_free(&even_comm);
    MPI_Comm_free(&odd_comm);
    MPI_Finalize();
    return 0;
}
