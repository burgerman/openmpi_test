#include <mpi.h>
#include <cstdio>
#include "string.h"
void print_processors() {
    int         my_rank;       /* rank of process      */
    int         p;             /* number of processes  */
    int         source;        /* rank of sender       */
    int         dest;          /* rank of receiver     */
    int         tag = 0;       /* tag for messages     */
    char        message[100];  /* storage for message  */
    MPI_Status  status;        /* status for receive   */

    // Initialize the MPI environment
    MPI_Init(nullptr, nullptr);

    /* Find out process rank  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank < p-1)
    {
        dest= my_rank + 1;
        /* Create message */
        sprintf(message, "Greetings from process %d to %d!",
               my_rank, dest);
        /* Use strlen+1 so that '\0' gets transmitted */
        MPI_Send(message, strlen(message)+1, MPI_CHAR,
                 dest, tag, MPI_COMM_WORLD);
    }
    else
    {
        dest = 0;
        /* Create message */
        sprintf(message, "Greetings from process %d to %d!",
                my_rank, dest);
        /* Use strlen+1 so that '\0' gets transmitted */
        MPI_Send(message, strlen(message)+1, MPI_CHAR,
                 dest, tag, MPI_COMM_WORLD);
    }
    if (my_rank < 1)
    {
        source = p-1;
        MPI_Recv(message, 100, MPI_CHAR, source, tag,
                     MPI_COMM_WORLD, &status);
        printf("%s\n", message);
    }
    else
    {
        source = my_rank-1;
        MPI_Recv(message, 100, MPI_CHAR, source, tag,
        MPI_COMM_WORLD, &status);
        printf("%s\n", message);
    }
    /* Shut down MPI */
    MPI_Finalize();
}