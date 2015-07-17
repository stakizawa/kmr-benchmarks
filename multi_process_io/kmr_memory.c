/*
    A synthetic benchmark for measuring data access performance.
*/
#include <stdio.h>
#include <mpi.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    printf("this is test.\n");
    MPI_Finalize();
    return 0;
}
