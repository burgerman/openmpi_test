//
// Created by Wilfried Wu on 2024-02-21.
//
#include "midterm_sample.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define TOTAL_POINTS 80000000

void part_b_algo() {
    int my_rank, p, value, rec_value, sum, tmp_sum;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(p<=1) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    } else if ((p & (p-1)) != 0) {
        printf("num of processes should be a power of 2");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int steps = (int) log2(p);
    value = my_rank;

    if(my_rank%2==0){
        MPI_Send(&value, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&rec_value, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(my_rank != 1) {
            MPI_Recv(&tmp_sum, 1, MPI_INT, my_rank-2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum = tmp_sum + value + rec_value;
        } else {
            sum = value + rec_value;
        }
        if(my_rank == (p/2 - 1)) {
            MPI_Send(&sum, 1, MPI_INT, p-1, 0, MPI_COMM_WORLD);
        } else if(my_rank != p-1) {
            MPI_Send(&sum, 1, MPI_INT, my_rank+2, 0, MPI_COMM_WORLD);
        } else {
            MPI_Recv(&tmp_sum, 1, MPI_INT, p/2 - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += tmp_sum;
            printf("sum result: %d\n", sum);
        }
    }

    MPI_Finalize();
}

void compute_pi_by_monte_carlo() {
    int rank, size, i;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(rank);

    int points_inside_circle = 0;

    for(i = 0; i< TOTAL_POINTS/size; i++){
        double x = (double) rand() / RAND_MAX;
        double y = (double) rand() / RAND_MAX;

        if(x*x + y*y <= 1) {
            points_inside_circle++;
        }
    }
    int total_points_inside_circle;
    if (rank == 0) {
        total_points_inside_circle = points_inside_circle;
        for (i=1; i<size; i++) {
            MPI_Recv(&points_inside_circle, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_points_inside_circle += points_inside_circle;
        }
        double pi = (double) (4.0 * total_points_inside_circle / TOTAL_POINTS);
        printf("PI is: %f\n", pi);
    } else {
        MPI_Send(&points_inside_circle, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
