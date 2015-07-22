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
calc_pair(int even_recv, int odd_recv, int rank, int nprocs,
	  int *even_send, int *odd_send)
{
    int *recv1 = (int *)malloc(sizeof(int) * nprocs);
    MPI_Allgather(&even_recv, 1, MPI_INT, recv1, 1, MPI_INT, MPI_COMM_WORLD);
    int *recv2 = (int *)malloc(sizeof(int) * nprocs);
    MPI_Allgather(&odd_recv, 1, MPI_INT, recv2, 1, MPI_INT, MPI_COMM_WORLD);
    _Bool setone = 0;
    for (int i = 0; i < nprocs; i++) {
	if (recv1[i] == rank) {
	    *even_send = i;
	    if (setone == 1) {
		break;
	    } else {
		setone = 1;
	    }
	}
	if (recv2[i] == rank) {
	    *odd_send = i;
	    if (setone == 1) {
		break;
	    } else {
		setone = 1;
	    }
	}
    }
    free(recv1);
    free(recv2);
}

static void
add_initial_data(common_t *common)
{
    char filename[FILENAME_LEN];
    create_file(common->rank, common->iteration, common->file_size,
		filename, FILENAME_LEN);
    common->val_count = IO_COUNT * common->file_size;
}

static void
increase_in_file_value(common_t *common, int recv_from, int send_to)
{
    char infile[FILENAME_LEN];
    snprintf(infile, FILENAME_LEN, "./%06d-%02d.dat", common->rank,
	     common->iteration);
    char outfile[FILENAME_LEN];
    snprintf(outfile, FILENAME_LEN, "./%06d-%02d.dat", common->rank,
	     common->iteration + 1);
    /* a huge buffer */
    long *buf = (long *)malloc(sizeof(long) * common->val_count);

    FILE *ifp = fopen(infile, "r");
    assert(ifp != 0);
    long *sbuf = (long *)malloc(sizeof(long) * IO_COUNT);
    long *rbuf = buf;
    /* send/recv 1MB at once */
    for (int i = 0; i < common->file_size; i++) {
	size_t cc = fread(sbuf, sizeof(long), IO_COUNT, ifp);
	assert(cc == IO_COUNT);
	MPI_Status stat;
	MPI_Sendrecv(sbuf, IO_COUNT, MPI_LONG, send_to, 1000,
		     rbuf, IO_COUNT, MPI_LONG, recv_from, 1000,
		     MPI_COMM_WORLD, &stat);
	rbuf += IO_COUNT;
    }
    free(sbuf);
    fclose(ifp);

#ifdef DEBUG
    fprintf(stderr, "Rank[%d]: process val[%ld]\n", common->rank, buf[0]);
#endif
    for (int i = 0; i < common->val_count; i++) {
	buf[i] += 1;
    }

    FILE *ofp = fopen(outfile, "w+");
    assert(ofp != 0);
    sbuf = buf;
    rbuf = (long *)malloc(sizeof(long) * IO_COUNT);
    /* send/recv 1MB at once */
    for (int i = 0; i < common->file_size; i++) {
	MPI_Status stat;
	MPI_Sendrecv(sbuf, IO_COUNT, MPI_LONG, recv_from, 1001,
		     rbuf, IO_COUNT, MPI_LONG, send_to, 1001,
		     MPI_COMM_WORLD, &stat);
	size_t cc = fwrite(rbuf, sizeof(long), IO_COUNT, ofp);
	assert(cc == IO_COUNT);
	sbuf += IO_COUNT;
    }
    free(rbuf);
    fclose(ofp);

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

    //MPI_Comm task_comm;
    int color = (rank + rank / task_nprocs * (task_nprocs - 1)) % task_nprocs;
    //MPI_Comm_split(MPI_COMM_WORLD, color, rank, &task_comm);

    int even_recv_from = color * task_nprocs + rank % task_nprocs;
    int odd_recv_from = (rank / task_nprocs) * task_nprocs + color;
    int even_send_to = -1;
    int odd_send_to = -1;
    calc_pair(even_recv_from, odd_recv_from, rank, nprocs,
	      &even_send_to, &odd_send_to);

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
	int recv_from = (i % 2 == 0) ? even_recv_from : odd_recv_from;
	int send_to = (i % 2 == 0) ? even_send_to : odd_send_to;
	increase_in_file_value(&common0, recv_from, send_to);
	struct timeval te;
	measure_time(&te);
	itr_times[i] = calc_time_diff(&ts, &te);
    }
    delete_file(common0.rank, common0.iteration + 1);

    print_time(itr_times, ITERATIONS, rank);

    //MPI_Comm_free(&task_comm);
    MPI_Finalize();
    return 0;
}
