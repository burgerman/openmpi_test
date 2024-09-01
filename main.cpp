/* #include "openmpi_test.h"
#include "mpicollective_test.h"
#include "trapezoidal_rule_test.h"
#include "broadcast_algo.h"
#include "assign1_test.h"
#include "assign1_partiii.h"
#include "datatype_test.h"
#include "matrix_io_test.h"
#include "cartesian_shift_test.h"
#include "midterm_sample.h"*/
#include "a2_sorting_parallel.h"
#include "a2_matrix_sum.h"
#include <stdio.h>
#include <mpi.h>
#define MAX_TIME 15.0

int main(int argc, char* argv[]) {
    int my_rank, p;
    /* Start up MPI */
    MPI_Init(&argc, &argv);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    // Analyze performance for different values of n
    int n = 8;
    double start_time = MPI_Wtime();
    double func_start_time, func_end_time, end_time;
    double time_cost;
    for(;;) {
        // Print the results
        func_start_time = MPI_Wtime();
        sort_n_in_parallel(n, my_rank, p);
        func_end_time = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);
        printf("When N is: %d;\n", n);
        printf("Parallel Time is: %f \n", (func_end_time-func_start_time));
        func_start_time = MPI_Wtime();
        sort_n_in_serial(n, my_rank, p);
        func_end_time = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);
        printf("When N is: %d;\n", n);
        printf("Serial Time is: %f \n", (func_end_time-func_start_time));
        end_time = MPI_Wtime();
        time_cost = (double) (end_time - start_time);
        if(time_cost>=MAX_TIME) {
            break;
        }
        n *=2;
    }
    /* Shut down MPI */
    MPI_Finalize();
    return 0;
}