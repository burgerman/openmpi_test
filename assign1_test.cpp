#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include <math.h>
#include <unistd.h>
//
// Created by Wilfried Wu on 2024/1/18.
//

//static float high = 512.0;
//static float low = -512.0;

static float high = (float)RAND_MAX;
static float low = -(float)RAND_MAX;

static float lowest_arr[3];

static float eggholder_func (float x, float y) {
    return -(y + 47.0) * sin(sqrt(fabs(x / 2.0 + (y + 47.0)))) - x * sin(sqrt(fabs(x - (y + 47.0))));
}

static void find_lowest_fvalue(float func_v, float x, float y) {
    if(func_v < lowest_arr[0]){
        lowest_arr[0] = func_v;
        lowest_arr[1] = x;
        lowest_arr[2] = y;
    }
}

static double time_limit = 30.0;

void my_terminator(int rank){
    printf("Clean up before terminating process %d\n", rank);
    /* Shut down MPI */
    MPI_Finalize();
    exit(0);
}

static int N = 5;

void assign1_part_i() {
    /* Start up MPI */
    MPI_Init(NULL, NULL);

    int my_rank, p, i;
    double start_time, end_time, time_cost;
    float rand_x_y[2];
    float x, y, result, f_value;
    float endpoint = INFINITY;
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    start_time=MPI_Wtime();

    srand(my_rank+getpid());
    for(i=0; i<N; i++) {
        rand_x_y[0] = low + ((float) rand()/ (float)RAND_MAX)* (high-low);
        rand_x_y[1] = low + ((float) rand()/ (float)RAND_MAX)* (high-low);
        result = eggholder_func(rand_x_y[0], rand_x_y[1]);
        if(result<endpoint){
            x = (float)rand_x_y[0];
            y = (float)rand_x_y[1];
            f_value = result;
            endpoint = result;
        }
    }
    float func_arr[3];
    if (my_rank != 0) {
        func_arr[0] = f_value;
        func_arr[1] = x;
        func_arr[2] = y;
        MPI_Send(func_arr, 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    } else {
        lowest_arr[0] = f_value;
        lowest_arr[1] = x;
        lowest_arr[2] = y;
        for(i=1; i<p; i++) {
            MPI_Recv(func_arr, 3, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            find_lowest_fvalue(func_arr[0], func_arr[1], func_arr[2]);
        }
        printf("Found lowest value : %f\n", lowest_arr[0]);
        printf("Generated X: %f\n", lowest_arr[1]);
        printf("Generated Y: %f\n", lowest_arr[2]);
    }
    end_time=MPI_Wtime();
    time_cost = end_time-start_time;
    printf("Time cost is : %f seconds\n", time_cost);
    if(time_cost>=time_limit){
        printf("Time cost up to 30 secs");
        MPI_Abort(MPI_COMM_WORLD, 0);
    }
    /* Shut down MPI */
    MPI_Finalize();
}


void test_time_check() {
    // Initialize the MPI environment
    MPI_Init(nullptr, nullptr);
    int my_rank;
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int p;
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    double starttime, endtime, time_cost;
    starttime=MPI_Wtime();
    int i=0;
    for(;;) {
        endtime = MPI_Wtime();
        time_cost = endtime-starttime;
        if(time_cost>=time_limit) {
//            MPI_Abort(MPI_COMM_WORLD, 0);
            printf("gracefully exit\n");
            printf("current value: %d at process %d\n", i, my_rank);
            my_terminator(my_rank);
        }
        i++;
    }
}

void assign1_part_ii() {
    int my_rank;
    int p;
    int i;
    float rand_x_y[2];
    float x, y;
    double start_time, end_time, time_cost;
    float local_lowest_arr[3];
    float func_arr[3];
    /* Start up MPI */
    MPI_Init(NULL, NULL);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // record the start time of computing
    start_time=MPI_Wtime();

    srand(my_rank+getpid());

    for(;;) {
        rand_x_y[0] = low + ((float) rand()/ (float)RAND_MAX)* (high-low);
        rand_x_y[1] = low + ((float) rand()/ (float)RAND_MAX)* (high-low);
        x = (float)rand_x_y[0];
        y = (float)rand_x_y[1];
        float f_value = eggholder_func(x, y);
        func_arr[0] = f_value;
        func_arr[1] = x;
        func_arr[2] = y;
        if (f_value<func_arr[0]) {
            func_arr[0] = f_value;
            func_arr[1] = x;
            func_arr[2] = y;
        }

        end_time = MPI_Wtime();
        // current time cost
        time_cost = end_time-start_time;
        if(time_cost>=time_limit) {
            printf("Time is up\n");
            if (my_rank != 0) {
                //send all results of processes to root process
                MPI_Send(func_arr, 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
            }
            else {
                float lowest_arr[3];
                // set current value of root process as base to compare
                lowest_arr[0] = local_lowest_arr[0];
                lowest_arr[1] = local_lowest_arr[1];
                lowest_arr[2] = local_lowest_arr[2];
                for(i=1; i<p; i++) {
                    MPI_Recv(local_lowest_arr, 3, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // compare the generated value of eggholder func
                    if(local_lowest_arr[0] < lowest_arr[0]){
                        lowest_arr[0] = local_lowest_arr[0];
                        lowest_arr[1] = local_lowest_arr[1];
                        lowest_arr[2] = local_lowest_arr[2];
                    }
                }
                printf("process : %d\n",my_rank);
                printf("Found lowest value : %f\n", lowest_arr[0]);
                printf("Generated X: %f\n", lowest_arr[1]);
                printf("Generated Y: %f\n", lowest_arr[2]);
            }
            break;
        }
    }
    /* Shut down MPI */
    MPI_Finalize();
}

static float time_check_interval = 4.0;
static float least_improve = 0.1;
static float lowest[3];

static void check_lowest_if_improved(float func_v, float x, float y, float *tmp_arr) {
    if(func_v < tmp_arr[0]){
        tmp_arr[0] = func_v;
        tmp_arr[1] = x;
        tmp_arr[2] = y;
    }
}

void test_partiii() {
    int my_rank;
    int p;
    int exit_flag = 0;
    float *rand_x_y = nullptr;
    float x, y;
    float f_value;
    float improved_v = (float)RAND_MAX;
    double start_time, current_time, time_cost;
    float func_arr[3];
    float tmp_arr[3];
    /* Start up MPI */
    MPI_Init(nullptr, nullptr);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    srand48(my_rank);

    // record the start time of computing
    start_time= MPI_Wtime();
    for(;;) {
//        MPI_Barrier(MPI_COMM_WORLD);
//        rand_x_y = generate_rand_nums(my_rank);
        x = (float)(2.0 * RAND_MAX * drand48()) - (float)RAND_MAX;
        y = (float)(2.0 * RAND_MAX * drand48()) - (float)RAND_MAX;
        f_value = eggholder_func(x, y);
        printf("Found lowest value : %f\n", f_value);
        printf("Current X: %f\n", x);
        printf("Current Y: %f\n", y);
        if(f_value<func_arr[0]) {
            func_arr[0] = f_value;
            func_arr[1] = x;
            func_arr[2] = y;
        }

        current_time = MPI_Wtime();
        time_cost = current_time-start_time;
        if(time_cost>=time_check_interval) {
            MPI_Send(func_arr, 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
            if(my_rank == 0) {
                // process 0
                tmp_arr[0] = func_arr[0];
                tmp_arr[1] = func_arr[1];
                tmp_arr[2] = func_arr[2];
                for(int i=1; i<p; i++) {
                    MPI_Recv(func_arr, 3, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if(func_arr[0] < tmp_arr[0]){
                        tmp_arr[0] = func_arr[0];
                        tmp_arr[1] = func_arr[1];
                        tmp_arr[2] = func_arr[2];
                    }
//                    check_lowest_if_improved(func_arr[0], func_arr[1], func_arr[2], tmp_arr);
                }
                improved_v = lowest[0] - tmp_arr[0];
                printf("improved by : %f\n", improved_v);
                if(improved_v < least_improve) {
                    exit_flag = 1;
                }
                else {
                    lowest[0] = tmp_arr[0];
                    lowest[1] = tmp_arr[1];
                    lowest[2] = tmp_arr[2];
                }
            }
        }
        if(my_rank!=0) {
            MPI_Recv(&exit_flag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else
        {
            for(int i=1; i<p; i++) {
                MPI_Send(&exit_flag, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (exit_flag == 1) {
            printf("Found lowest value : %f\n", lowest[0]);
            printf("Current X: %f\n", lowest[1]);
            printf("Current Y: %f\n", lowest[2]);
            break;
        }
        else
        {
            start_time = current_time;
        }
//        MPI_Barrier(MPI_COMM_WORLD);
//        sleep(time_check_interval);
    }
    /* Shut down MPI */
    MPI_Finalize();
}