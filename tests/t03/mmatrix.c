#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define OUTPUT_FILE "timing_data.txt"

MPI_Status status;

int main (int argc, char **argv) {

    int rank, num_procs; // process ID, number of processes
    int num_slaves; // number of processes excluding root processes
    int source, dest; //message passing source and destination
    int num_rows, offset; //num of rows given to each processes, stating row index

    double start_time1, end_time1, t1; //t1: calculation time of the root process
    double start_time2, end_time2, t2; //t2: calculation time of the each process
    double start_time, end_time, t; //t: execution time of the program

    int N = atoi(argv[1]); //matrix size passed as an argument in command line

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    double tarr [num_procs]; //array of calculation times of all processes
    int idx = 0;

    //Ensure there are enogh processors
    if (rank == 0 && num_procs < 2) {

        fprintf(stderr, "At least 2 processors are required\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    num_slaves = num_procs - 1 ;

    //Allocate matrices dinamically
    int (*A)[N] = malloc(N * N * sizeof(int));
    int (*B)[N] = malloc(N * N * sizeof(int));
    int (*C)[N] = malloc(N * N * sizeof(int)); //output matrix C = A X B

    // Root process
    if (rank == 0) {

        srand(time(NULL)); //to get different inputs on different runs

        //Fill the input matrix A
        for (int i = 0; i < N; i++) {

            for (int j = 0; j < N; j++) {

                A[i][j] = 1 + rand() % 9;
            }
        }

        //Fill the input matrix B
        for (int i = 0; i < N; i++) {

            for (int j = 0; j < N; j++) {

                B[i][j] = 1 + rand() % 9;
            }
        }

        //Print the input matrix A
        printf("\nMatrix A\n\n");
        for (int i = 0; i < N; i++) {

            for (int j = 0; j < N; j++) {

                printf("%d ", A[i][j]);
            }
            printf("\n");
        }

        //Print the input matrix B
        printf("\nMatrix B\n\n");
        for (int i = 0; i < N; i++) {

            for (int j = 0; j < N; j++) {

                printf("%d ", B[i][j]);
            }
            printf("\n");
        }

        start_time = MPI_Wtime();
        //Calculate the total number of rows each process should receive
        num_rows = N / num_procs; //each process gets an equal share of total rows
        offset = 0; //distribution starts from row 0

        //Allotting calculation tasks to slaves
        printf("\n");
        for (dest = 1; dest <= num_slaves; dest++) {

            MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
            MPI_Send(&num_rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
            MPI_Send(&A[offset][0], num_rows * N, MPI_INT, dest, 1, MPI_COMM_WORLD);
            printf("Rows from %d to %d are allotted to process %d\n",  offset, offset + num_rows - 1, dest);
            MPI_Send(&B[0][0], N * N, MPI_INT, dest, 1, MPI_COMM_WORLD);
            offset += num_rows;
        }

        printf("Rows from %d to %d are allotted to process 0.\n",  offset, offset + num_rows - 1);

        // Receiving task outputs
        printf("\n");
        for (int i = 1; i <= num_slaves; i++ ) {

            source = i;
            MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&num_rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&C[offset][0], num_rows * N, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&t2, 1, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
            tarr[idx] = t2;
            idx++;
            printf("Process %d sent task ouputs %d to %d\n", source, C[offset][0], C[offset][num_rows * N - 1]);
        }

        //Calculating its own part of output
        start_time1 = MPI_Wtime();
        for (int k = 0; k < N; k++) {

            for (int i = offset + 1; i < N; i++) {

                C[i][k] = 0;
                for (int j = 0; j < N; j++) {

                    C[i][k] += A[i][j] * B[j][k];
                }
            }
        }

        end_time1 = MPI_Wtime();
        t1 = end_time1 - start_time1;
        tarr[idx] = t1;

        //Finding the max of the calculation times
        double max = tarr[0];
        for (int z = 1; z < num_procs; z++) {

            if (tarr[z] >= max) {

                max = tarr[z];
            }
        }

        end_time = MPI_Wtime();
        t = end_time - start_time;

        //Printing final output
        printf("\nResult Matrix C = Matrix A * Matrix B:\n\n");
        for (int i = 0; i < N; i++) {

            for (int j = 0; j < N; j++) {
                printf("%d\t", C[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        //Copy the timing data into a file
        FILE *fp = fopen(OUTPUT_FILE, "a");
        if (fp == NULL) {

            fprintf(stderr, "Failed to open file for writting\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fprintf(fp, "np = %d, n = %d, et = %f, ct = %f\n", num_procs, N, t,  max);
        fclose(fp);
        free(A);
        free(B);
        free(C);
    }
    //Slave processes
    else {

        source = 0;
        MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&num_rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&A[0][0], num_rows * N, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&B[0][0], N * N, MPI_INT, source, 1, MPI_COMM_WORLD, &status);

        start_time2 = MPI_Wtime();

        //Calculating tasks outputs

        for (int k = 0; k < N; k++) {

            for (int i = 0; i < num_rows; i++) {

                C[offset + i][k] = 0;
                for (int j = 0; j < N; j++) {

                    C[offset + i][k] += A[i][j] * B[j][k];
                }
            }
        }

        end_time2 = MPI_Wtime();
        t2 = end_time2 - start_time2;

        //Sending task outputs to root process
        MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&num_rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&C[offset][0], num_rows * N, MPI_INT, 0, 2, MPI_COMM_WORLD);

        //Sending calculation time of each process to the root process
        MPI_Send(&t2, 1, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}