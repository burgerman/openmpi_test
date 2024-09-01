//
// Created by Wilfried Wu on 2024-02-27.
//

#ifndef OPENMPI_TEST_A2_SORTING_PARALLEL_H
#define OPENMPI_TEST_A2_SORTING_PARALLEL_H
void sort_n_in_parallel(int N, int my_rank, int p);
void sort_n_in_serial(int N, int my_rank, int p);
void sort_by_partition(int N, int my_rank, int p);
#endif //OPENMPI_TEST_A2_SORTING_PARALLEL_H
