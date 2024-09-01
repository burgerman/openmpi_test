//
// Created by Wilfried Wu on 2024/2/1.
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

typedef struct {
    int row;
    int col;
    int value;
} Diagonals;

static int n = 64;

void datatype_struct_create() {
    int my_rank, p;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int matrix[n][n];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = ((i * n + j) + (my_rank*n*n));
        }
    }
    MPI_Datatype diagonal_type;
    MPI_Aint displacements[] = {0, offsetof(Diagonals, col), offsetof(Diagonals, value)};
    int block_lengths[] = {1, 1, 1};
    MPI_Datatype types[] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Type_create_struct(3, block_lengths, displacements, types, &diagonal_type);
    MPI_Type_commit(&diagonal_type);
    if (my_rank == 0) {
        Diagonals send_diagonal_elements;
        // Process 0 Sends

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d ", matrix[i][j]);
            }
            printf("\n");
        }
        printf("matrix on process 0 is as above");
        for (int i = 0; i < n; i++) {
            send_diagonal_elements.row = i;
            send_diagonal_elements.col = i;
            send_diagonal_elements.value = matrix[i][i];
            MPI_Send(&send_diagonal_elements, 1, diagonal_type, 1, 0, MPI_COMM_WORLD);
        }
    }
    else if(my_rank == 1) {
        printf("matrix on process 1 is as below\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d ", matrix[i][j]);
            }
            printf("\n");
        }
        Diagonals recv_diagonal_elements;
        // Process 1 receives
        printf("Received diagonal elements:\n");
        for (int i = 0; i < n; i++) {
            MPI_Recv(&recv_diagonal_elements, 1, diagonal_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            matrix[i][i] = recv_diagonal_elements.value;
            printf("%d ", matrix[i][i]);
        }
        printf("\n");
        printf("updated matrix on process 1 is as below\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d ", matrix[i][j]);
            }
            printf("\n");
        }
    }
    MPI_Type_free(&diagonal_type);
    MPI_Finalize();
}

void datatype_indexed_create() {
    int my_rank, p;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int matrix[n*n];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i*n+j] = ((i * n + j) + (my_rank*n*n));
        }
    }
    MPI_Datatype indexed_diagonal_type;
    int block_lengths[n];
    for(int i =0; i<n; i++) {
        block_lengths[i] = 1;
    }
    int displacements[n];
    displacements[0] = 0;
    for(int i=1; i<n; i++) {
        displacements[i] = i*n+i;
    }
    MPI_Type_indexed(n, block_lengths, displacements, MPI_INT, &indexed_diagonal_type);
    MPI_Type_commit(&indexed_diagonal_type);
    if (my_rank == 0) {
        MPI_Send(matrix, 1, indexed_diagonal_type, 1, 0, MPI_COMM_WORLD);
    }
    else if(my_rank == 1) {
        int recv_vec[n];
        // Process 1 receives
        printf("Process 1 received diagonal elements:\n");
        MPI_Recv(recv_vec, 7, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=0; i<n; i++) {
            printf("%d ", recv_vec[i]);
        }
    }
    MPI_Type_free(&indexed_diagonal_type);
    MPI_Finalize();
}

static char filename[] = "matrix_output.txt";

void optimized_question_a() {
    int my_rank, p, i, j, proc, col;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(n<p || n%p !=0) {
        printf("The num of n must be equal to or greater than the num of processes");
        printf("The num of n must be divided evenly by the num of processes");
        MPI_Abort(MPI_COMM_WORLD, 0);
    }
    // Create the matrix
    int matrix[n][n];
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            matrix[i][j] = i * n + j;  // Some example data
        }
    }
    int blocks = n/p;
    // Define a column-based data type
    MPI_Datatype column_based_type;
    MPI_Type_vector(n, 1, n, MPI_INT, &column_based_type);
    MPI_Type_commit(&column_based_type);

    for(col=0; col<blocks; col++) {
        // Send columns to processes
        MPI_Send(&matrix[0][my_rank*blocks+col], 1, column_based_type, 0, 0, MPI_COMM_WORLD);
    }
    // Print the received column
    if (my_rank == 0) {
        int received_matrix[n][n];
        for(proc=0; proc<p; proc++) {
            for(col=0; col<blocks; col++) {
                MPI_Recv(&received_matrix[0][proc*blocks+col], 1, column_based_type, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                printf("%d ", received_matrix[i][j]);
            }
            printf("\n");
        }
        printf("Columns received by process 0:\n");
        FILE *file = fopen(filename, "w+");
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                fprintf(file, "%d ", received_matrix[i][j]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }

    MPI_Type_free(&column_based_type);
    MPI_Finalize();
}



void optimized_question_b() {
    int my_rank, p, i, j, proc;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    int blocks = n/p;
    if(n<p || n%p !=0) {
        printf("The num of n must be equal to or greater than the num of processes");
        printf("The num of n must be divided evenly by the num of processes");
        MPI_Abort(MPI_COMM_WORLD, 0);
    }
    // Create the matrix
    int **matrix = NULL;
    int **received_matrix = NULL;
    if(my_rank == 0) {
        matrix = (int **)malloc(n * sizeof(int *));
        for (i = 0; i < n; i++) {
            matrix[i] = (int *)malloc(n * sizeof(int));
        }
        // Open the file
        FILE *file = fopen(filename, "r");
        if (file != NULL) {
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    // Read in a formatted way
                    fscanf(file, "%d", &matrix[i][j]);
                }
            }
            // Close the file
            fclose(file);
        }
        else {
            printf("File Cannot be found/opened.\n");
        }

        for(i=my_rank*blocks; i<my_rank*blocks+blocks; i++) {
            for(j=0; j<n; j++){
                printf("%d ", matrix[j][i]);
            }
            printf("\n");
        }
        printf("data on process %d as above\n", my_rank);
    }

    // Define a column-based data type
    MPI_Datatype column_based_type;
    MPI_Type_vector(n, blocks, n, MPI_INT, &column_based_type);
    MPI_Type_commit(&column_based_type);
    if (my_rank == 0) {
        for(proc=1; proc<p; proc++) {
            // Send columns to processes
            MPI_Send((matrix[0]+proc*blocks), 1, column_based_type, proc, 0, MPI_COMM_WORLD);
        }
    } else {
        received_matrix = (int **)malloc(n * sizeof(int *));
        for (i = 0; i < n; i++) {
            received_matrix[i] = (int *)malloc(n * sizeof(int));
        }
        MPI_Recv(received_matrix[0]+proc*blocks, blocks, column_based_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(i=proc*blocks; i<proc*blocks+blocks; i++) {
            for(j=0; j<n; j++){
                printf("%d ", received_matrix[j][i]);
            }
            printf("\n");
        }
        printf("received by process %d \n", my_rank);
    }
    if(my_rank==0) {
        for (i = 0; i < n; i++) {
            free(matrix[i]);
        }
        free(matrix);
    } else{
        for (i = 0; i < n; i++) {
            free(received_matrix[i]);
        }
        free(received_matrix);
    }
    MPI_Type_free(&column_based_type);
    MPI_Finalize();
}

