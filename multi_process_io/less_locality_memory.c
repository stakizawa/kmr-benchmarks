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
add_initial_data(long *data, common_t *common)
{
    for (int i = 0; i < common->val_count; i++) {
	data[i] = common->rank;
    }
}

static void
increase_in_memory_value(long *din, long *dout, common_t *common,
			 int recv_from, int send_to)
{
    long *buf = (long *)malloc(sizeof(long) * common->val_count);
    MPI_Status stat;
    MPI_Sendrecv(din, common->val_count, MPI_LONG, send_to, 1000,
		 buf, common->val_count, MPI_LONG, recv_from, 1000,
		 MPI_COMM_WORLD, &stat);

#ifdef DEBUG
    fprintf(stderr, "Rank[%d]: process val[%ld]\n",
    	    common->rank, buf[0]);
#endif
    for (int i = 0; i < common->val_count; i++) {
	buf[i] += 1;
    }

    MPI_Sendrecv(buf, common->val_count, MPI_LONG, recv_from, 1001,
		 dout, common->val_count, MPI_LONG, send_to, 1001,
		 MPI_COMM_WORLD, &stat);
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
    parse_param(argc, argv, &(common0.val_count));
    common0.rank = rank;
    long *data0 = (long *)malloc(sizeof(long) * common0.val_count);
    add_initial_data(data0, &common0);

    double itr_times[ITERATIONS];
    for (int i = 0; i < ITERATIONS; i++) {
	struct timeval ts;
	measure_time(&ts);
	int recv_from = (i % 2 == 0) ? even_recv_from : odd_recv_from;
	int send_to = (i % 2 == 0) ? even_send_to : odd_send_to;
	long *data1 = (long *)malloc(sizeof(long) * common0.val_count);
	increase_in_memory_value(data0, data1, &common0, recv_from, send_to);
	free(data0);
	struct timeval te;
	measure_time(&te);
	itr_times[i] = calc_time_diff(&ts, &te);

	data0 = data1;
    }
    free(data0);

    print_time(itr_times, ITERATIONS, rank);

    //MPI_Comm_free(&task_comm);
    MPI_Finalize();
    return 0;
}
