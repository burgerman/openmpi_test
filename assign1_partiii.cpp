//
// Created by Wilfried Wu on 2024/1/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>


static float high = (float)RAND_MAX;
static float low = (float)-RAND_MAX;
static float time_check_interval = 4.0;
static float least_improve = 0.1;

static float eggholder_func (float x, float y) {
    return -(y + 47.0) * sin(sqrt(fabs(x / 2.0 + (y + 47.0)))) - x * sin(sqrt(fabs(x - (y + 47.0))));
}

void assign1_part_iii() {

    float lowest[3];
    int my_rank;
    int p;
    int i;
    int exit_flag = 0;
    float x, y;
    float f_value;
    float improved_v;
    double start_time, current_time, time_cost;
    float func_arr[3];
    float tmp_arr[3];
    /* Start up MPI */
    MPI_Init(NULL, NULL);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    srand(my_rank);
    // record the start time of computing
    start_time= MPI_Wtime();
    // endless loop
    for(;;) {
        // randomly generate x and y
        x = low + ((float) rand()/ (float)RAND_MAX)* (high-low);
        y = low + ((float) rand()/ (float)RAND_MAX)* (high-low);
        // computing the value of the eggholder function
        f_value = eggholder_func(x, y);
        // Store the recent least value in func_arr for each process
        func_arr[0] = f_value;
        func_arr[1] = x;
        func_arr[2] = y;
        current_time = MPI_Wtime();
        time_cost = current_time-start_time;
        // periodically check
        if(time_cost>=time_check_interval) {
            if(my_rank == 0) {
                // the value of process 0
                tmp_arr[0] = func_arr[0];
                tmp_arr[1] = func_arr[1];
                tmp_arr[2] = func_arr[2];
                // receive the recent least value of eggholder from other processes
                float recv_func_arr[3];
                for(i=1; i<p; i++) {
                    MPI_Recv(recv_func_arr, 3, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if(recv_func_arr[0] < tmp_arr[0]){
                        tmp_arr[0] = recv_func_arr[0];
                        tmp_arr[1] = recv_func_arr[1];
                        tmp_arr[2] = recv_func_arr[2];
                    }
                }
                // Check the improvement
                improved_v = lowest[0] - tmp_arr[0];
                printf("improved by : %f\n", improved_v);
                if(improved_v < least_improve) {
                    // Improved by less than 0.1, exit.
                    exit_flag = 1;
                }
                else {
                    // Improved by more than 0.1, continue.
                    lowest[0] = tmp_arr[0];
                    lowest[1] = tmp_arr[1];
                    lowest[2] = tmp_arr[2];
                }
                printf("Latest lowest value : %f\n", lowest[0]);
                printf("X: %f\n", lowest[1]);
                printf("Y: %f\n", lowest[2]);
                printf("improved by : %f\n", improved_v);
                for(i=1; i<p; i++) {
                    // synchronize the status of exit_flag with other processes
                    MPI_Send(&exit_flag, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }
            }
            else {
                // all other processes send func_arr to process 0
                MPI_Send(func_arr, 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
                // receive signal from process 0 to see if the process should exit
                MPI_Recv(&exit_flag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            if (exit_flag == 1) {
                // safely exit loop
                break;
            }
            //Synchronize before the time recording updates
            start_time = current_time;
        }
    }
    /* Shut down MPI */
    MPI_Finalize();
}