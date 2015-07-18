/*
    A synthetic benchmark for measuring data access performance.

    Number of processes to run this program should be N^2, upto 10,000
    (N = 100).
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <kmr.h>

#define VAL_COUNT 10

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
    if (argc == 2) {
        *val_count = atoi(argv[1]);
    }
}

typedef struct {
    int key;
    int val_count;
    int rank;
} keyval_t;

/* map function */
static int
add_initial_data(const struct kmr_kv_box kv,
                 const KMR_KVS *kvi, KMR_KVS *kvo, void *p, long i_)
{
    keyval_t *keyval = (keyval_t *)p;
    long *val = (long *)malloc(sizeof(long) * keyval->val_count);
    for (int i = 0; i < keyval->val_count; i++) {
        val[i] = keyval->rank;
    }
    struct kmr_kv_box nkv = { .klen = sizeof(long),
                              .vlen = sizeof(long) * keyval->val_count,
                              .k.i = keyval->key,
                              .v.p = (void *)val };
    kmr_add_kv(kvo, nkv);
    return MPI_SUCCESS;
}


int
main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int nprocs, rank, task_nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    check_nprocs(nprocs, rank, &task_nprocs);
    kmr_init();
    KMR *mr = kmr_create_context(MPI_COMM_WORLD, MPI_INFO_NULL, 0);

    int even_key = rank / task_nprocs + 1;
    int odd_key = rank % task_nprocs + 1;

    keyval_t keyval;
    keyval.key = even_key;
    keyval.val_count = VAL_COUNT;
    parse_param(argc, argv, &(keyval.val_count));
    keyval.rank = rank;
    KMR_KVS *kvs0 = kmr_create_kvs(mr, KMR_KV_INTEGER, KMR_KV_OPAQUE);
    kmr_map_once(kvs0, &keyval, kmr_noopt, 0, add_initial_data);

    /* main loop */

    kmr_free_context(mr);
    kmr_fin();
    MPI_Finalize();
    return 0;
}
