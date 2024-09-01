//
// Created by Wilfried Wu on 2024/1/15.
//

#include <mpi.h>
#include <cstdio>
#include <memory>
#include <cstdlib>
#include <assert.h>
#include <time.h>
#include <thread>

double *create_rand_nums(int num) {
    double *randnums = (double *)malloc(sizeof(double) * num);
    assert(randnums != nullptr);
    for (int i = 0; i<num; i++) {
        randnums[i] = (rand() / (double)RAND_MAX) * 10;
    }
    return randnums;
}

double compute_avg(double *sub_nums, int nums) {
    double sum = 0.0;
    for(int i=0; i<nums; i++) {
        sum += sub_nums[i];
    }
    return sum / (double) nums;
}

void print_avg(int num_per_proc) {
    srand(time(nullptr));
    double all_avg;
    // Initialize the MPI environment
    MPI_Init(nullptr, nullptr);
    int my_rank;
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int p;
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    double *rand_nums = nullptr;
    /* if it is the root process */
    if(my_rank == 0) {
        rand_nums =create_rand_nums(num_per_proc * p);
        all_avg = compute_avg(rand_nums,
                                    num_per_proc * p);
        printf("The mean of all elements is %f\n", all_avg);
    }
    double *sub_nums = (double *)malloc(sizeof(double) * num_per_proc);
    assert(sub_nums!=nullptr);
    // allocate numbers from the root process to other processes
    MPI_Scatter(rand_nums, num_per_proc, MPI_DOUBLE,
                sub_nums, num_per_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double sub_avg = compute_avg(sub_nums, num_per_proc);
    printf("the sub_avg is: %.6f\n", sub_avg);
    double *sub_avgs = nullptr;
    if(my_rank == 0) {
        sub_avgs = (double *) malloc(sizeof(double) * p);
        assert(sub_avgs != nullptr);
    }
    MPI_Gather(&sub_avg, 1, MPI_DOUBLE, sub_avgs,
               1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
//    MPI_Allgather(&sub_avg, 1, MPI_DOUBLE, sub_avgs, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    if(my_rank == 0) {
        double avg = compute_avg(sub_avgs, p);
        printf("The parallel-computing-based mean of all elements is %f\n", avg);
    }

    /* clean up allocated memory on the root process*/
    if(my_rank == 0) {
        free(rand_nums);
        free(sub_avgs);
        rand_nums = nullptr;
        sub_avgs = nullptr;
    }
    free(sub_nums);
    sub_nums= nullptr;
    /* Blocks until all processes in the communicator have reached this routine*/
    MPI_Barrier(MPI_COMM_WORLD);
    /* Shut down MPI */
    MPI_Finalize();
}
