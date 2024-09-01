//
// Created by Wilfried Wu on 2024/1/16.
//
#include <iostream>
#include <mpi.h>

double func(double x) {
    return 4.0 / (1.0 + x*x);
}
/* the formula of trapezoidal area: (f(x0)+f(x1))*(x1-x0)/2
 * f(x0) is left side of trapezoid
 * f(x1) is right side of trapezoid
 * (x1-x0) is the height of trapezoid represented by h
 * (f(x0) + f(x1)) *h /2
 * */
double trapezoidalrule(int n, int rank, int p) {
    double a = 0.0, b=1.0;
    double h = (b-a) / n;
    double local_sum = 0.0;
    /* Trapezoid starts from the left endpoint, so i starts from 0 to n-1 */
    for (int i =rank; i<n; i += p) {
        /* left endpoint of trapezoid: a+i*h */
        double left_endpoint = a+i*h;
        /* right endpoint of trapezoid: a+(i+1)*h */
        double right_endpoint = a+(i+1)*h;
        local_sum += (func(left_endpoint)+ func(right_endpoint))*h/2;
    }
    return local_sum;
}

void print_area(int n) {
    MPI_Init(nullptr, nullptr);
    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    double localSum = trapezoidalrule(n, rank, p);
    double all_sum;
    if(rank == 0) {
        all_sum = localSum;
        for (int i=1; i<p; i++) {
            double recv_sum;
            MPI_Recv(&recv_sum, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            all_sum += recv_sum;
        }
    } else {
        MPI_Send(&localSum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    if (rank==0) {
        printf("With N = %d trapezoids", n);
        printf("Approximation of pi using trapezoidal rule is : %.9f\n", all_sum);
    }
    MPI_Finalize();
}
