//
// Created by Wilfried Wu on 2024/1/16.
//
#include <mpi.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>


int get_largest_power_of_2(int p) {
    int largest_power_of_2;
    if((p&(p-1))!=0) {
        int posNum = 0;
        while((1u << posNum) <=p) {
            posNum++;
        }
        largest_power_of_2 = 1u << (posNum-1);
    }
    else {
        largest_power_of_2 = p;
    }
    return largest_power_of_2;
}

void my_broadcast_test() {
    int         my_rank;       /* rank of process      */
    int         p;             /* number of processes  */
    int broadcast_integer; /*integer for broadcasting */
    /* Start up MPI */
    MPI_Init(nullptr, nullptr);

    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    broadcast_integer=-1;
    if (my_rank == 0) broadcast_integer=10;
    /* find the largest power of 2 less than or equal to the number of processes */
    int largest_power_of_2 = get_largest_power_of_2(p);
    int group_size = largest_power_of_2;
    MPI_Group world_group, sub_group;
    MPI_Comm group_comm = MPI_COMM_NULL;
    int ranks[group_size];
    for (int i = 0; i < group_size; i++) {
        ranks[i] = i;
    }
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, group_size, ranks, &sub_group);
    MPI_Comm_create_group(MPI_COMM_WORLD, sub_group, 0, &group_comm);
    int prime_rank, prime_size;
    MPI_Bcast(&broadcast_integer, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if (MPI_COMM_NULL != group_comm) {
        broadcast_integer=100;
        MPI_Comm_rank(group_comm, &prime_rank);
        MPI_Comm_size(group_comm, &prime_size);
        MPI_Bcast(&broadcast_integer, 1, MPI_INT, 0, group_comm);
    }
    if (prime_rank!=0) {
        printf("bc_data: %d. ", broadcast_integer);
    }
    else if(my_rank!=0) {
        printf("bc_data: %d. ", broadcast_integer);
    }

    /* Shut down MPI */
    MPI_Finalize();

    printf("Process %d has integer %d \n",my_rank, broadcast_integer);
}


void broadcast_algo() {
    int         my_rank;       /* rank of process      */
    int         p;             /* number of processes  */
    int         source;        /* rank of sender       */
    int         dest;          /* rank of receiver     */
    int         tag = 0;       /* tag for messages     */
    MPI_Status  status;        /* status for receive   */
    int broadcast_integer; /*integer for broadcasting */
    int spacing; /*distance between sending processes*/
    int stage; /* stage of algorithm */

    /* Start up MPI */
    MPI_Init(nullptr, nullptr);

    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    broadcast_integer=-1;
    if (my_rank == 0) broadcast_integer=100;
    /* find the largest power of 2 less than or equal to the number of processes */
    int largest_power_of_2;
    if((p&(p-1))!=0) {
        int posNum = 0;
        while((1u << posNum) <=p) {
            posNum++;
        }
        largest_power_of_2 = 1u << (posNum-1);
    }
    else {
        largest_power_of_2 = p;
    }
    spacing = largest_power_of_2;
    stage=0;
    while (spacing>1){
        if (my_rank>largest_power_of_2-1)
        {
            MPI_Send(&broadcast_integer, 1, MPI_INT, MPI_PROC_NULL, 0, MPI_COMM_WORLD);
        }

        else if (my_rank % spacing == 0)
        {
            dest = my_rank+spacing/2;
            printf("Process %d, sending message to process %d at stage %d \n",my_rank,dest,stage);
            MPI_Send(&broadcast_integer, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        else  if (my_rank % (spacing/2) == 0)
        {
            source=my_rank-spacing/2;
            printf("Process %d, receive message from process %d at stage %d \n",my_rank,source,stage);
            MPI_Recv(&broadcast_integer, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
        }
//      uncomment for debugging
//        sleep(1);
        MPI_Barrier(MPI_COMM_WORLD);

        spacing=spacing/2;
        stage=stage+1;
    }
    /* Shut down MPI */
    MPI_Finalize();

    printf("Process %d has integer %d \n",my_rank,broadcast_integer);
}
