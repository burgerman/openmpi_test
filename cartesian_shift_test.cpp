//
// Created by Wilfried Wu on 2024/2/5.
//

#include "mpi.h"
#include "stdio.h"
#include "cartesian_shift_test.h"

void carte_shift_test() {
    MPI_Init(nullptr, nullptr);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Create a 2D Cartesian grid communicator
    int dims[2] = {2, 2}; // 2x2 grid
    int periods[2] = {1, 1}; // Periodic boundary conditions in both dimensions
    MPI_Comm comm_cart;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &comm_cart);

    // Perform shifts along rows (dimension 0) and columns (dimension 1)
    int source, dest;

    // Shift along rows (dimension 0)  from left to right
    MPI_Cart_shift(comm_cart, 0, 1, &source, &dest);
    printf("Process %d: source = %d, destination = %d (shift right along rows)\n", rank, source, dest);

    // Shift along columns (dimension 1) from up to down
    MPI_Cart_shift(comm_cart, 1, 1, &source, &dest);
    printf("Process %d: source = %d, destination = %d (shift down along columns)\n", rank, source, dest);

    MPI_Finalize();
}