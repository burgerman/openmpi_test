//
// Created by Wilfried Wu on 2024/1/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define MYFILE "mpi_io.txt"


static char filename[] = "matrix_example.txt";


static void matrix_part_i(int n) {
    int my_rank;
    int p;
    /* Start up MPI */
    MPI_Init(nullptr, nullptr);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    int my_size = n/p;
    int sub_matrix[my_size][n];
    for(int i=0; i<my_size; i++) {
        for(int j=0; j<n; j++) {
            sub_matrix[i][j] = (int)(rand() / RAND_MAX) * (n*n);
        }
    }
    // define a derived datatype for p2p communication
//    MPI_Datatype derived_datatype;
//    MPI_Type_vector(block_size, n, N * size, MPI_INT, &block_type);
//    MPI_Type_commit(&block_type);


    if(my_rank == 0) {
        MPI_File matrix_file;
        MPI_File_open(MPI_COMM_NULL, filename, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &matrix_file);
        // Write the local portion of the matrix to the file
        MPI_File_write(matrix_file, sub_matrix, my_size * n, MPI_INT, MPI_STATUS_IGNORE);
        // Close the file
        MPI_File_close(&matrix_file);
    }
    else
    {
        MPI_Send(sub_matrix, my_size * n, MPI_INT, 0, 0,MPI_COMM_WORLD );

    }

    /* Shut down MPI */
    MPI_Finalize();
}

static void load_matrix (int** matrix, int n) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fscanf(file, "%d", &matrix[i][j]);
            }
        }
        // Close the file
        fclose(file);
        // Display the matrix
        printf("Matrix read from the file:\n");
    } else {
        printf("File Cannot be found/opened.\n");
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("------------Matrix loaded as above------------------");
}


static void matrix_question_a(int n) {
    int my_rank, p;
    /* Start up MPI */
    MPI_Init(nullptr, nullptr);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // num of rows for each process
    int num_row = n / p;

    /* row-based approach to transferring data between processes */
    // create a sub matrix for each process
    int *sub_matrix = (int *)malloc(num_row * n * sizeof(int));

    // set values of sub matrices for all processes
    for (int i = 0; i < num_row * n; i++) {
        sub_matrix[i] = my_rank * num_row * n + i;
    }

    // Create a derived datatype for the block
    MPI_Datatype derived_datatype;
    // Number of rows for each derived datatype
    int derived_datatype_size = n / p;
    MPI_Type_vector(derived_datatype_size, num_row * n, 0, MPI_INT, &derived_datatype);
    MPI_Type_commit(&derived_datatype);

    if (my_rank != 0) {
        // If not process 0, Send a vector of data to process 0
        MPI_Send(sub_matrix, 1, derived_datatype, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        /* process 0 receives data */
        // create a dynamic array for matrix
        int *my_matrix = (int *)malloc(n * n * sizeof(int));

        // Copy data of process 0 to the matrix
        for (int i = 0; i < derived_datatype_size; i++) {
            for (int j = 0; j < n; j++) {
                my_matrix[i * n + j] = sub_matrix[i * n + j];
                printf("%d ", my_matrix[i * n + j]);
            }
        }
        printf("\n");
        printf("------------process 0 above------------------\n");
        // Receive data from other processes
        for (int source = 1; source < p; source++) {
            MPI_Recv(my_matrix + source * derived_datatype_size * n, 1, derived_datatype, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int j=0; j<derived_datatype_size * n; j++){
                printf("%d ", my_matrix[source * derived_datatype_size * n+j]);
            }
            printf("\n");
        }
        printf("----------------other processes above-----------------------------\n");
        FILE *file = fopen(filename, "w+");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fprintf(file, "%d ", my_matrix[i * n + j]);
                printf("%d ", my_matrix[i * n + j]);
            }
            fprintf(file, "\n");
            printf("\n");
        }
        fclose(file);
        printf("----------------matrix above-----------------------------\n");
        // clean up derived datatype objects
        MPI_Type_free(&derived_datatype);
        // clean up allocated array
        free(my_matrix);
    }
    // clean up sub matrix for all processes
    free(sub_matrix);

    /* Shut down MPI */
    MPI_Finalize();
}

void parallel_write_file() {
    MPI_Init(NULL, NULL);
    int rank, size;
    int i;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_File  file;
    MPI_Status status;

    int datasize = 20;

    MPI_File_open(MPI_COMM_WORLD, MYFILE, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    MPI_Offset displacement = (int) (rank * datasize * sizeof(int));
    MPI_File_set_view(file, displacement, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);

    int *dataset;
    dataset = (int *) malloc(datasize * sizeof(int));

    for(i = 0; i<datasize; i++){
        dataset[i] = rank*datasize + i;
    }
    MPI_File_write(file, &dataset, datasize, MPI_INT, &status);

    MPI_File_close(&file);
    free(dataset);
    MPI_Finalize();
}


void parallel_read_file() {
    MPI_Init(NULL, NULL);
    int rank, size;
    int i;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_File  file;
    MPI_Status status;

    int datasize = 20;
    int *dataset;
    dataset = (int *) malloc(datasize * sizeof(int));

    MPI_File_open(MPI_COMM_WORLD, MYFILE, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_Offset displacement = (int) (rank * datasize * sizeof(int));
    MPI_File_set_view(file, displacement, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);


    MPI_File_read(file, dataset, datasize, MPI_INT, &status);

    MPI_File_close(&file);

    printf("Process %d read data: ", rank);
    for(i = 0; i<datasize; i++){
        dataset[i] = rank*datasize + i;
        printf("%d ", dataset[i]);
    }
    printf("\n");
    free(dataset);
    MPI_Finalize();
}


void matrix_question_b(int n) {
    int my_rank, p;
    /* Start up MPI */
    MPI_Init(nullptr, nullptr);
    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // num of rows for each process
    int num_row = n / p;

    // create a sub matrix for each process
    int *sub_matrix = (int *)malloc(num_row * n * sizeof(int));

    // Create a derived datatype for the block
    MPI_Datatype derived_datatype;
    // Number of rows for each derived datatype
    int derived_datatype_size = n / p;
    MPI_Type_vector(1,  num_row*n, 0, MPI_INT, &derived_datatype);
    MPI_Type_commit(&derived_datatype);

    if(my_rank == 0) {
        /* Load the matrix into array*/
        int **matrix = (int **)malloc(n * sizeof(int *));
        for (int i = 0; i < n; i++) {
            matrix[i] = (int *)malloc(n * sizeof(int));
        }

        // Open the file
        FILE *file = fopen(filename, "r");
        if (file != NULL) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    // Read in a formatted way
                    fscanf(file, "%d", &matrix[i][j]);
                }
            }
            // Close the file
            fclose(file);
        } else {
            printf("File Cannot be found/opened.\n");
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                // Read in a formatted way
                printf("%d ", matrix[i][j]);
            }
            printf("\n");
        }
        printf("-----------matrix loaded as above----------------------------------\n");
        /* distribute a sub matrix to each of the processes: */
        for (int proc = 0; proc<p; proc++) {
            int r = 0;
            for (int i = proc*num_row; i < num_row + proc*num_row; i++) {
                for(int j=0; j<n; j++) {
                    sub_matrix[r * n + j] = matrix[i][j];
                }
                r++;
            }
            printf("---------------data sent from process 0 to process %d is as below----------------\n", proc);
            for(int i=0; i<num_row*n; i++) {
                printf("%d ", sub_matrix[i]);
            }
            printf("\n");
            MPI_Send(sub_matrix, 1, derived_datatype, proc, 0, MPI_COMM_WORLD);
        }
        for (int i = 0; i < n; i++) {
            free(matrix[i]);
        }
        free(matrix);
    }
    // receive sub matrix data from process 0
    MPI_Recv(sub_matrix, 1, derived_datatype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // print each sub_matrix by row
    printf("---------------data received from process 0 to process %d is as below----------------\n", my_rank);
    for (int i=0; i<num_row; i++){
        for(int j = 0; j<n; j++){
            printf("%d ", sub_matrix[i*n+j]);
        }
        printf("\n");
    }
    printf("---------------splitter----------------\n");
    free(sub_matrix);
    MPI_Type_free(&derived_datatype);
    /* Shut down MPI */
    MPI_Finalize();
}



void analyze_question_a (int n, int my_rank, int p) {
    int i, j, source;

    // num of rows for each process
    int num_row = n / p;

    /* row-based approach to transferring data between processes */
    // create a sub matrix for each process
    int *sub_matrix = (int *)malloc(num_row * n * sizeof(int));

    // set values of sub matrices for all processes
    for (i = 0; i < num_row * n; i++) {
        sub_matrix[i] = my_rank * num_row * n + i;
    }

    // Create a derived datatype for the block
    MPI_Datatype derived_datatype;
    // Number of rows for each derived datatype
    int derived_datatype_size = n / p;
    MPI_Type_vector(derived_datatype_size, num_row * n, 0, MPI_INT, &derived_datatype);
    MPI_Type_commit(&derived_datatype);

    if (my_rank != 0) {
        // If not process 0, Send a vector of data to process 0
        MPI_Send(sub_matrix, 1, derived_datatype, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        /* process 0 receives data */
        // create a dynamic array for matrix
        int *my_matrix = (int *)malloc(n * n * sizeof(int));

        // Copy data of process 0 to the matrix
        for (i = 0; i < derived_datatype_size; i++) {
            for (j = 0; j < n; j++) {
                my_matrix[i * n + j] = sub_matrix[i * n + j];
                printf("%d ", my_matrix[i * n + j]);
            }
        }
        printf("\n");
        printf("------------------process 0 above------------------\n");
        // Receive data from other processes
        for (source = 1; source < p; source++) {
            MPI_Recv(my_matrix + source * derived_datatype_size * n, 1, derived_datatype, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(j=0; j<derived_datatype_size * n; j++){
                printf("%d ", my_matrix[source * derived_datatype_size * n+j]);
            }
            printf("\n");
        }
        printf("-----------------------------other processes above-----------------------------\n");
        FILE *file = fopen(filename, "w+");
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                fprintf(file, "%d ", my_matrix[i * n + j]);
                printf("%d ", my_matrix[i * n + j]);
            }
            fprintf(file, "\n");
            printf("\n");
        }
        fclose(file);
        printf("-----------------------------matrix above-----------------------------\n");
        // clean up derived datatype objects
        MPI_Type_free(&derived_datatype);
        // clean up allocated array
        free(my_matrix);
    }
    // clean up sub matrix for all processes
    free(sub_matrix);
}


void analyze_question_b (int n, int my_rank, int p) {
    int i, j, r, proc;
    // num of rows for each process
    int num_row = n / p;

    int **matrix = NULL;
    if(my_rank == 0) {
        // create a sub matrix for sending n/p * n/p matrix each round
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
    }
    int divd = 1;
    int chunk_size = num_row * n;
    int small_chunk_size = chunk_size / divd;

    // Create a derived datatype for the block
    MPI_Datatype derived_datatype;
    // Number of rows for each derived datatype
    int derived_datatype_size = num_row*n;
    MPI_Type_vector(1,  small_chunk_size, 0, MPI_INT, &derived_datatype);
    MPI_Type_commit(&derived_datatype);

    int *sd_chunk = NULL;

    if(my_rank == 0) {
        sd_chunk = (int *)malloc(small_chunk_size * sizeof(int));
        for (r=0; r<divd; r++) {
            for (i = 0; i < num_row / divd; i++) {
                for (j = 0; j < n; j++) {
                    printf("%d ", matrix[(0 * (num_row / divd) + r * (p * num_row / divd)) + i][j]);
                }
                printf("\n");
            }
        }
        printf("\n---------------data of process 0 is as above----------------\n");
    }

    int *small_chunk = (int *)malloc(small_chunk_size * sizeof(int));

    for (int r=0; r<divd; r++) {
        if(my_rank == 0) {
            /* distribute a sub matrix to each of the processes: */
            for(proc = 1; proc<p; proc++) {
                for (i = 0; i < num_row/divd; i++) {
                    for (j = 0; j < n; j++) {

                        sd_chunk[i*n+j] = matrix[(proc*(num_row/divd)+ r * (p * num_row / divd)) + i][j];
                    }
                }
                MPI_Send(sd_chunk, 1, derived_datatype, proc, 0, MPI_COMM_WORLD);
            }
        }
        else{
            MPI_Recv(small_chunk, 1, derived_datatype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (i=0; i<num_row/divd; i++){
                for(j=0; j<n; j++) {
                    printf("%d ", small_chunk[i*n + j]);
                }
                printf("\n");
            }
            printf("\n---------------data received from process 0 to process %d is as above----------------\n", my_rank);
        }
    }

    if(my_rank == 0) {
        free(sd_chunk);
        for (i = 0; i < n; i++) {
            free(matrix[i]);
        }
        free(matrix);
    }
    free(small_chunk);
    MPI_Type_free(&derived_datatype);
}