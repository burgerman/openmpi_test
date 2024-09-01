//
// Created by Wilfried Wu on 2024-02-27.
//
#include "a2_sorting_parallel.h"
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

/*
 *  The non-recursive implementation of merge sort mainly relies on the partitioning of sub-sequences:
 *Firstly, you need a temporary array and the intervals of the left and right sub-sequences.
 * Starting from an interval length of 1 (doubling each time), divide the original array into pairs of left and right sub-sequences.
 * Sequentially compare and store the pairs of left right sub-sequences into the temporary array.
 * Sort the sequences in the temporary array and then copy them back to the original array.
 * Finally, double the interval length and repeat the above operations, which is the merging process.
 *  time complexity: O(n log n)
 *  The solution from: https://blog.nowcoder.net/n/8a21b62939064b698cb6b68ee944c50d
 * */

void merge_sort(float num[],int len, int N) {
    int i,j;
    float *sort;
    int L_start=0,L_end=0;//left partition end indices
    int R_start=0,R_end=0;//right partition end indices

    sort=(float*)malloc(N*sizeof(float));//tmp array

    for(i=1;i<N;i*=2)//increase by double
    {
        for(L_start=0;L_start<len-i;L_start=R_end)
        {
            L_end   = L_start+i;
            R_start = L_start+i;
            R_end   = R_start+i;

            if(R_end>len)//right end no more than length of arr
            {
                R_end=len;
            }
            j=0;    //init index of tmp arr

            while(L_start<L_end && R_start<R_end)
            {
                // compare start points of left and right, put smaller one into tmp arr in order
                if(num[L_start]<num[R_start])
                    sort[j++]=num[L_start++];
                else
                    sort[j++]=num[R_start++];
                //start index increases
            }

            while(L_start<L_end)//put all the rest of elements into the tmp arr after the comparison
            {
                sort[j++]=num[L_start++];
            }
            while(j>0)           //put sorted elements of tmp arr into original arr
            {
                num[--R_start]=sort[--j];
            }
        }
    }
    free(sort);
}


void make_ledger(int my_ledger[], float arr[], int p, int N) {
    int i, j;
    for(i=0; i<N; i++) {
        j = (int)(arr[i]*p);
        my_ledger[j] = my_ledger[j] + 1;
    }
}


void sort_n_in_parallel(int N, int my_rank, int p) {
    int i, j, k;
    MPI_Status status;
    MPI_Request requests[2*p*p];
    srand(getpid()+my_rank);
    float *local_nums = (float *)malloc(N * sizeof(float));
    for (i=0; i<N; i++) {
        local_nums[i] = (float) rand()/ (float)RAND_MAX;
    }

    // Merge sorting from https://blog.nowcoder.net/n/8a21b62939064b698cb6b68ee944c50d
    merge_sort(local_nums, 0, N - 1);
    // Count the num of elements in each partition
    int *ledger = (int *)malloc(p * sizeof(int));
    make_ledger(ledger, local_nums, p, N);

    // divide sorted elements into the corresponding partitions
    // num of partitions equals to num of processes
    float ** partitioned_nums = (float **) malloc(p* sizeof(float *));
    int current_index = 0;
    for(i = 0; i<p; i++) {
        partitioned_nums[i] = (float *) malloc(ledger[i] * sizeof(float));
        for(j =0; j<ledger[i]; j++) {
            partitioned_nums[i][j] = local_nums[current_index];
            current_index++;
        }
    }
    int size_of_entries_to_receive[p];//count the num of elements of a specific partition across processes
    int total_num_of_entries = 0;//total num of elements of a partition on the my_rank process
    float *total_part_nums;//store all elements of a partition across processes

    // receive elements of a partition from processes
    float ** received_partitioned_nums = (float **) malloc(p* sizeof(float *));
    for (i=0; i<p; i++) {
        // start data integration from partition 0 to partition (p-1)
       if (my_rank == i) {
            size_of_entries_to_receive[i] = ledger[i];
            for(j=0; j<p; j++) {
                if( j != i) {
                    MPI_Irecv(size_of_entries_to_receive+j, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &requests[i*2*p+j]);
                    MPI_Wait(&requests[i*2*p+j], &status);
                }
            }
            for(k=0; k<p; k++) {
                total_num_of_entries += size_of_entries_to_receive[k];
            }
            total_part_nums = (float *) malloc(total_num_of_entries * sizeof(float));
            for(j=0; j<p; j++) {
                received_partitioned_nums[j] = (float *) malloc(size_of_entries_to_receive[j] * sizeof(float));
                if( j != i) {
                    MPI_Irecv(received_partitioned_nums[j], size_of_entries_to_receive[j], MPI_FLOAT, j, 0, MPI_COMM_WORLD, &requests[i*2*p+p+j]);
                    MPI_Wait(&requests[i*2*p+p+j], &status);
                } else {
                    // if dest process is the current process, copy data into its corresponding partition
                    for(k=0; k<ledger[j]; k++) {
                        received_partitioned_nums[j][k] = partitioned_nums[j][k];
                    }
                }
            }
            int local_index = 0;
            for (j=0; j<p; j++) {
                for(k=0; k<size_of_entries_to_receive[j]; k++) {
                    total_part_nums[local_index] = received_partitioned_nums[j][k];
                    local_index++;
                }
            }
            for (k = 0; k < p; k++) {
                free(received_partitioned_nums[k]);
            }
        } else {
            MPI_Isend(&ledger[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i*2*p+my_rank]);
            MPI_Wait(&requests[i*2*p+my_rank], &status);
            MPI_Isend(partitioned_nums[i], ledger[i], MPI_FLOAT, i, 0 , MPI_COMM_WORLD, &requests[i*2*p+p+my_rank]);
            MPI_Wait(&requests[i*2*p+p+my_rank], &status);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    if( total_num_of_entries != 0) {
        // merge Sorting
        merge_sort(total_part_nums, 0, total_num_of_entries-1);
    }
    free(received_partitioned_nums);
    for (k = 0; k < p; k++) {
        free(partitioned_nums[k]);
    }
    free(partitioned_nums);
    free(ledger);
    free(local_nums);
    free(total_part_nums);
}



void sort_n_in_serial(int N, int my_rank, int p) {
    int i, j, k;
    MPI_Status status;
    srand(getpid()+my_rank);
    float *local_nums = (float *)malloc(N * sizeof(float));
    for (i=0; i<N; i++) {
        local_nums[i] = (float) rand()/ (float)RAND_MAX;
    }
    merge_sort(local_nums, 0, N - 1);
    float *total_nums;
    int *ledger = (int *)malloc(p * sizeof(int));;
    float ** partitioned_nums;
    float *received_partition_nums;

    if(my_rank == 0) {
        total_nums = (float *)malloc(N* p * sizeof(float));
        for(k=0; k<N; k++) {
            total_nums[k] = local_nums[k];
        }
        for(i=1; i<p; i++) {
            MPI_Recv(total_nums+i*N, N, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &status);
        }
        merge_sort(total_nums, 0, N* p - 1);
        make_ledger(ledger, total_nums, p, N*p);
        partitioned_nums = (float **) malloc(p* sizeof(float *));
        int current_index = 0;
        for(i = 0; i<p; i++) {
            partitioned_nums[i] = (float *) malloc(ledger[i] * sizeof(float));
            for(j =0; j<ledger[i]; j++) {
                partitioned_nums[i][j] = total_nums[current_index];
                current_index++;
            }
        }
    } else {
        MPI_Send(local_nums, N, MPI_FLOAT, 0, 0 , MPI_COMM_WORLD);
    }
//    MPI_Bcast(ledger, p, MPI_INT, 0, MPI_COMM_WORLD);
//    if(my_rank != 0) {
//        received_partition_nums = (float *) malloc(ledger[my_rank] * sizeof(float));
//        MPI_Recv(received_partition_nums, ledger[my_rank], MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
//    } else {
//        for(j=1; j<p; j++) {
//            MPI_Send(partitioned_nums[j], ledger[j], MPI_FLOAT, j, 0 , MPI_COMM_WORLD);
//        }
//    }
    if(my_rank == 0) {
        for(i=1; i<p; i++) {
            MPI_Send(&ledger[i], 1, MPI_INT, i, 0 , MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&ledger[my_rank], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    if(my_rank == 0) {
        for(i=1; i<p; i++) {
            MPI_Send(partitioned_nums[i], ledger[i], MPI_FLOAT, i, 0 , MPI_COMM_WORLD);
        }
    } else {
        received_partition_nums = (float *) malloc(ledger[my_rank] * sizeof(float));
        MPI_Recv(received_partition_nums, ledger[my_rank], MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
    }

    if(my_rank == 0) {
        free(total_nums);
        for (k = 0; k < p; k++) {
            free(partitioned_nums[k]);
        }
        free(partitioned_nums);
    } else {
        free(received_partition_nums);
    }
//    free(received_partition_nums);
    free(local_nums);
}



void sort_by_partition (int N, int my_rank, int p) {
    int i, j, k;
    MPI_Status status;
    MPI_Request requests[2*p*p];
    srand(getpid()+my_rank);
    float *local_nums = (float *)malloc(N * sizeof(float));
    for (i=0; i<N; i++) {
        local_nums[i] = (float) rand()/ (float)RAND_MAX;
    }

//    float key;
    // Insertion sorting from https://www.geeksforgeeks.org/insertion-sort/
//    for (i = 1; i < N; i++) {
//        key = local_nums[i];
//        j = i - 1;
//        while (j >= 0 && local_nums[j] > key) {
//            local_nums[j + 1] = local_nums[j];
//            j = j - 1;
//        }
//        local_nums[j + 1] = key;
//    }
    merge_sort(local_nums, N, N);

    int *ledger = (int *)malloc(p * sizeof(int));
    int index;
    for(i=0; i<N; i++) {
        index = (int)(local_nums[i]*p);
        ledger[index] = ledger[index] + 1;
    }

    float ** partitioned_nums = (float **) malloc(p* sizeof(float *));
    int current_index = 0;
    for(i = 0; i<p; i++) {
        partitioned_nums[i] = (float *) malloc(ledger[i] * sizeof(float));
        for(j =0; j<ledger[i]; j++) {
            partitioned_nums[i][j] = local_nums[current_index];
            current_index++;
        }
    }
    int size_of_entries_to_receive[p];
    int total_num_of_entries = 0;
    float *total_part_nums;

    float ** received_partitioned_nums = (float **) malloc(p* sizeof(float *));

    for (i=0; i<p; i++) {
//        printf("Process %d contains %d elements ranging between (%d /p) and (%d /p + 1/p)\n", my_rank, ledger[i], i, i);
        if (my_rank == i) {
            size_of_entries_to_receive[i] = ledger[i];
            for(j=0; j<p; j++) {
                if( j != i) {
                    MPI_Irecv(size_of_entries_to_receive+j, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &requests[i*2*p+j]);
                    MPI_Wait(&requests[i*2*p+j], &status);
                }
            }

            for(k=0; k<p; k++) {
                total_num_of_entries += size_of_entries_to_receive[k];
            }
//            printf("Process %d contains %d elements ranging between (%d /p) and (%d /p + 1/p)\n", my_rank, total_num_of_entries, my_rank, my_rank);
            total_part_nums = (float *) malloc(total_num_of_entries * sizeof(float));

            for(j=0; j<p; j++) {
                received_partitioned_nums[j] = (float *) malloc(size_of_entries_to_receive[j] * sizeof(float));
                if( j != i) {
                    MPI_Irecv(received_partitioned_nums[j], size_of_entries_to_receive[j], MPI_FLOAT, j, 0, MPI_COMM_WORLD, &requests[i*2*p+p+j]);
                    MPI_Wait(&requests[i*2*p+p+j], &status);
                } else {
                    // if dest process is the current process, copy data into its corresponding partition
//                    memcpy(received_partitioned_nums[j], partitioned_nums[j], ledger[j]*sizeof(float));
                    for(k=0; k<ledger[j]; k++) {
                        received_partitioned_nums[j][k] = partitioned_nums[j][k];
                    }
                }
            }
            current_index = 0;
            for (j=0; j<p; j++) {
                for(k=0; k<size_of_entries_to_receive[j]; k++) {
                    total_part_nums[current_index] = received_partitioned_nums[j][k];
                    current_index++;
                }
            }
            for (k = 0; k < p; k++) {
                free(received_partitioned_nums[k]);
            }
        } else {
            MPI_Isend(&ledger[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i*2*p+my_rank]);
            MPI_Wait(&requests[i*2*p+my_rank], &status);
            MPI_Isend(partitioned_nums[i], ledger[i], MPI_FLOAT, i, 0 , MPI_COMM_WORLD, &requests[i*2*p+p+my_rank]);
            MPI_Wait(&requests[i*2*p+p+my_rank], &status);
        }

//        MPI_Barrier(MPI_COMM_WORLD);
    }

    if( total_num_of_entries != 0) {
        // Insertion Sorting
//        for (k = 1; k < total_num_of_entries; k++) {
//            key = total_part_nums[k];
//            j = k - 1;
//            while (j >= 0 && total_part_nums[j] > key) {
//                total_part_nums[j + 1] = total_part_nums[j];
//                j = j - 1;
//            }
//            total_part_nums[j + 1] = key;
//        }
        merge_sort(total_part_nums, total_num_of_entries, total_num_of_entries);

//        printf("Sorted elements ranging between (%d/p) and (%d/p + 1/p) on process %d:\n", my_rank, my_rank, my_rank);
//        for (i = 0; i < total_num_of_entries; i++) {
//            printf("%f ", total_part_nums[i]);
//        }
//        printf("\n");
    }

    free(total_part_nums);
    free(received_partitioned_nums);
    for (k = 0; k < p; k++) {
        free(partitioned_nums[k]);
    }
    free(partitioned_nums);
    free(ledger);
    free(local_nums);
}