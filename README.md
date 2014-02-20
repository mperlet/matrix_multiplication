# Hochleistungsrechnen Assignment 1 

##### Parallel Matrix Multiplication Using OpenMP, Phtreads, and MPI 

----------

## Assignment
The multiplication of two matrices is to be implemented as 

* a sequential program 
* an OpenMP shared memory program 
* an explicitly threaded program (using the pthread standard) 
* a message passing program using the MPI standard 

## Matrix multiplication
The aim is to multiply two matrices together.To multiply two matrices, the number of columns of the first matrix has to match the number of lines of the second matrix. The calculation of the matrix solution has independent steps, it is possible to parallelize the calculation.

## Project Tree

    .
    |-- bin
    |   |-- mpi
    |   |-- omp
    |   |-- seq
    |   `-- thread2
    |-- data
    |   |-- mat_4_5.txt
    |   `-- mat_5_4.txt
    |-- src
    |   |-- matrix.c
    |   |-- matrix.h
    |   |-- mpi.c
    |   |-- omp.c
    |   |-- sequential.c
    |   |-- thread2.c
    |   `-- thread.c
    |-- Makefile
    |-- random_float_matrix.py
    |-- README.md
    |-- README.pdf
    `-- Test-Script.sh

The `README.*` contains this document as a Markdown and a PDF file.
The python script `random_float_matrix.py` generates `n x m` float matrices (This script is inspired by Philip Böhm's solution).
`./Test-Script.sh` is a script that generates test matrices with the python script, compiles the C-programs with `make` and executes the diffrent binaries with the test-matrices. The output of the script are the execution times of the particular implementations.

## Makefile
    CC=gcc
    CFLAGS= -Wall -std=gnu99 -g -fopenmp
    LIBS=src/matrix.c
    TUNE= -O2
    
    all: sequential omp thread2 mpi
    
    sequential:
    		$(CC) $(TUNE) $(CFLAGS) -o bin/seq $(LIBS) src/sequential.c
    
    omp:
    		$(CC) $(TUNE) $(CFLAGS) -o bin/omp $(LIBS) src/omp.c
    
    thread:
    		$(CC) $(TUNE) $(CFLAGS) -pthread -o bin/thread $(LIBS) src/thread.c
    
    thread2:
    		$(CC) $(TUNE) $(CFLAGS) -pthread -o bin/thread2 $(LIBS) src/thread2.c
    
    mpi:
    		mpicc $(TUNE) $(CFLAGS) -o bin/mpi $(LIBS) src/mpi.c

`make` translates all implementations. The binary files are then in the `bin/` directory.
The implementation `thread2.c` is the final solution of the *thread* subtask. `thread.c` was my first runnable solution but it is not fast(every row has one thread). I decided to keep it anyway, for a comparable set.
For the compiler optimization I have chosen "-02", the execution time was best here.

## Example
Every implementation needs 2 matrix files as program argument to calculate the result matrix to `stdout` (`bin/seq mat_file_1.txt mat_file_2.txt`).
The `rows` are seperated by newlines(`\n`) and the columns are seperated by tabular(`\t`). The reason is the pretty output on the shell. All implementations calculate with floating-point numbers.

    [mp432@localhost]% cat data/mat_4_5.txt 
    97.4549968447	4158.04953246	2105.6723138	9544.07472156	2541.05960201
    1833.23353473	9216.3834844	8440.75797842	1689.62403742	4686.03507194
    5001.05053096	7289.39586628	522.921369146	7057.57603906	7637.9829023
    737.191477364	4515.30312019	1370.71005027	9603.48073923	7543.51110732
    
    [mp432@localhost]% cat data/mat_5_4.txt 
    8573.64127861	7452.4636398	9932.62634628	1261.340226
    7527.08499112	3872.81522875	2815.39747607	5735.65492468
    7965.24212592	7428.31976294	290.255638815	5940.92582147
    6175.98390217	5995.21703679	6778.73998746	9060.90690747
    2006.95378498	6098.70324661	619.384482373	1396.62426963
    
    [mp432@localhost]% bin/seq data/mat_4_5.txt data/mat_5_4.txt
    112949567.256707	105187212.450287	79556423.335490    126508582.287008	
    172162416.208937	150764506.000392	60962563.539173    127174399.969315	
    160826865.507086	158278548.934611	122920214.859773   125839554.344572	
    125675943.680898	136743486.943968	90204309.448167	   132523052.230353	

## Implementations

### Sequential

The sequential program is used to compare and correctness to the other implementations. The following is an excerpt from the source code. Here is computed the result matrix.

    for (int i = 0; i < result_matrix->rows; i++) {
        for (int j = 0; j < result_matrix->cols; j++) {
            for (int k = 0; k < m_1->cols; k++) {
                result_matrix->mat_data[i][j] += m_1->mat_data[i][k] *      
                m_2->mat_data[k][j];
            }
        }
    }

### Thread (POSIX Threads)
The `sysconf(_SC_NPROCESSORS_ONLN)` from `#include <unistd.h>` returns the number of processors, what is set as the thread number, to use the full capacity. The following excerpt shows the thread memory allocation.

    int number_of_proc = sysconf(_SC_NPROCESSORS_ONLN);
    ...
    // Allocate thread handles
    pthread_t *threads;
    threads = (pthread_t *) malloc(number_of_proc * sizeof(pthread_t));

### Open Multi-Processing (OpenMP)
The standard shared-memory model is the fork/join model.
The OpenMP implementation is just the sequential program with the omp pragma `#pragma omp parallel for` over the first for-loop. This pragma can only be used in the outer loop. Only there are independent calculations.
The performance increased about 40 percent compared to the sequential implementation. 

    #pragma omp parallel for
    for (int i = 0; i < result_matrix->rows; i++) {
        for (int j = 0; j < result_matrix->cols; j++) {
            for (int k = 0; k < m_1->cols; k++) {
                result_matrix->mat_data[i][j] += m_1->mat_data[i][k] *
                m_2->mat_data[k][j];
            }
        }
    }
    
### Message Passing Interface (MPI)
A difficulty it was the spread of the data to the worker.
At first, the matrix dimensions will be broadcast via `MPI_Bcast(&matrix_properties, 4, MPI_INT, 0, MPI_COMM_WORLD);` to the workers.

The size of the matrices is fixed. Now the 2-Dim matrix is converted into a 1-Dim matrix. So it is easier and safer to distribute the matrix data.

This function gets a matrix struct and returns an 1-Dim data array.

    double *mat_2D_to_1D(matrix_struct *m) {
        double *matrix = malloc( (m->rows * m->cols) * sizeof(double) );
        for (int i = 0; i < m->rows; i++) {
            memcpy( matrix + (i * m->cols), m->mat_data[i], m->cols * sizeof(double) );
        }
        return matrix;
    }

The second step is to broadcast the matrix data to the workers. Each worker computes its own "matrix area" with the mpi `rank`. Disadvantage of this implementation is that first all the data are distributed.
The third step is to collect the data via 

    MPI_Gather(result_matrix, number_of_rows, 
               MPI_DOUBLE, final_matrix,
               number_of_rows,  MPI_DOUBLE,
               0, MPI_COMM_WORLD);`

At the end, the master presents the result matrix.

> To compile and run the mpi implementation, it is necessary that `mpicc` and `mpirun` are in the search path. (e.g. `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/openmpi/lib/  `)


## Performance Test
The `sirius cluster` was not available during task processing (specifically for the MPI program). Therefore, all performance tests were run on `atlas`.

    [mp432@atlas Parallel-Matrix-Multiplication-master]$ ./Test-Script.sh 
    generate test-matrices with python if no test data found
    
    generate 5x4 matrix...
    generate 100x100 matrix...
    generate 1000x1000 matrix...
    generate 5000x5000 matrix...
    compile...
    
    gcc -O2 -Wall -std=gnu99 -g -fopenmp -o bin/seq src/matrix.c src/sequential.c
    gcc -O2 -Wall -std=gnu99 -g -fopenmp -o bin/omp src/matrix.c src/omp.c
    gcc -O2 -Wall -std=gnu99 -g -fopenmp -pthread -o bin/thread2 src/matrix.c src/thread2.c
    mpicc -O2 -Wall -std=gnu99 -g -fopenmp -o bin/mpi src/matrix.c src/mpi.c
    
    calculate...
    
    * * * * * * * 100x100 Matrix
    with sequential    0m0.032s
    with omp           0m0.034s
    with thread2       0m0.032s
    with mpi(4p)       0m1.242s

    * * * * * * * 1000x1000 Matrix
    with sequential    0m11.791s
    with omp           0m4.182s
    with thread2       0m4.153s
    with mpi(4p)       0m12.682s
    
    * * * * * * * 5000x5000 Matrix
    with sequential    26m52.342s
    with omp           4m57.186s
    with thread2       5m5.767s
    with mpi(4p)       5m2.174s
    
The output times are the `real times` from the unix `time` command. 
You can see the advantages of parallel computation in the last matrix calculation. The parallel calculation is about 5 times faster (for large matrices).
