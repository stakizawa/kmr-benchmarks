/*
    A synthetic benchmark for measuring data access performance.
*/
#include <stdio.h>
#include <mpi.h>
#include <kmr.h>

int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    kmr_init();
    printf("this is test.\n");
    kmr_fin();
    MPI_Finalize();
    return 0;
}
