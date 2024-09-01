cmake_minimum_required(VERSION 3.27)
project(openmpi_test)

set(CMAKE_CXX_STANDARD 17)
set(MPI_C_COMPILER "/Users/wilfried/homebrew/bin/mpicc")
set(MPI_CXX_COMPILER "/Users/wilfried/homebrew/bin/mpicxx")
find_package(MPI REQUIRED)

add_executable(openmpi_test main.cpp openmpi_test.cpp
        mpicollective_test.cpp
        trapezoidal_rule_test.cpp
        broadcast_algo.cpp
        assign1_test.cpp
        assign1_partiii.cpp
        matrix_io_test.cpp
        deadlock_test.cpp
        datatype_test.cpp
        cartesian_shift_test.cpp
        midterm_sample.cpp
        a2_sorting_parallel.cpp
        a2_matrix_sum.cpp)
target_link_libraries(openmpi_test PRIVATE MPI::MPI_CXX)