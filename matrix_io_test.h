//
// Created by Wilfried Wu on 2024/1/21.
//

#ifndef OPENMPI_TEST_MATRIX_IO_TEST_H
#define OPENMPI_TEST_MATRIX_IO_TEST_H
//void matrix_part_i();
//void matrix_question_a(int n);
//void matrix_question_b(int n);
void parallel_write_file();
void parallel_read_file();
void analyze_question_a(int n, int my_rank, int p);
void analyze_question_b(int n, int my_rank, int p);
#endif //OPENMPI_TEST_MATRIX_IO_TEST_H
