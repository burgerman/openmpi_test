//
// Created by Wilfried Wu on 2024/1/24.
//
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

void test_deadlock() {

    int my_rank, p;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(p!=2) {
        printf("This program is supposed to run on two processes");
        MPI_Finalize();
    }
    int small_size = 256 * sizeof(int);
    int msg_size = 256 * 1024 * 256;
//    int *msg = (int *)(malloc(small_size));
//    int *flag = (int *)(malloc(small_size));
    for (int msg_s = 1024; msg_s < msg_size; msg_s *= 2) {
        int msg[msg_s];
        int flag[small_size];
        for (int i=0; i<msg_s; i++) {
            msg[i] = i;
        }
        if (my_rank == 0) {
            MPI_Send(msg, msg_s, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(flag, msg_s, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else
        {
            MPI_Send(flag, msg_s, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(msg, msg_s, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

    }
    MPI_Finalize();
}
