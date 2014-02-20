#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "matrix.h"

double *mat_2D_to_1D(matrix_struct *m) {
    double *matrix = malloc( (m->rows * m->cols) * sizeof(double) );
    for (int i = 0; i < m->rows; i++) {
        memcpy( matrix + (i * m->cols), m->mat_data[i], m->cols * sizeof(double) );
    }
    return matrix;
}

int main(int argc, char *argv[]) {
    /** Matrix Properties
     * [0] = Rows of Matrix A
     * [1] = Cols of Matrix A
     * [2] = Rows of Matrix B
     * [3] = Cols of Matrix B
     **/
    int matrix_properties[4];
     
    double *m_a = NULL;
    double *m_b = NULL;
    double *final_matrix = NULL;
    
    int num_worker, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_worker);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    /** the master initializes the data **/
    if (rank == 0) {
        
        if(argc != 3){
            printf("ERROR: Please specify only 2 files.\n");
            exit(EXIT_FAILURE);
        }
            
        matrix_struct *m_1 = get_matrix_struct(argv[1]);
        matrix_struct *m_2 = get_matrix_struct(argv[2]);

        if(m_1->cols != m_2->rows){
            printf("ERROR: The number of columns of matrix A must match the number of rows of matrix B.\n");
            exit(EXIT_FAILURE);
        }
        
        if (m_1->rows % num_worker != 0) {
            printf("ERROR: Matrix can not be calculated with this number of tasks.\n");
            exit(EXIT_FAILURE);
        }
        
        // fill the property-array for workers
        matrix_properties[0] = m_1->rows;
        matrix_properties[1] = m_1->cols;
        matrix_properties[2] = m_2->rows;
        matrix_properties[3] = m_2->cols;
        
        /* generate 1D matrices for workers 
         * m_a is the 1D Matrix of m_1 
         * m_a is the 1D Matrix of m_1 
        */
        m_a = mat_2D_to_1D(m_1);
        m_b = mat_2D_to_1D(m_2);

        free_matrix(m_1);
        free_matrix(m_2);
    }

    // send the matrix properties to the workers
    MPI_Bcast(&matrix_properties, 4, MPI_INT, 0, MPI_COMM_WORLD);

    // calculate the 1D-sizes of the matrices
    int size_a   = matrix_properties[0] * matrix_properties[1];
    int size_b   = matrix_properties[2] * matrix_properties[3];
    int size_res = matrix_properties[0] * matrix_properties[3];
    
    // allocate memory for 1D-matrices
    if(rank == 0) {
        final_matrix = malloc( size_res * sizeof(double) );
    } else {
        m_a = malloc( size_a * sizeof(double) );
        m_b = malloc( size_b * sizeof(double) );
    }

    // send 1D matrices to workers
    MPI_Bcast(m_a, size_a , MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(m_b, size_b , MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // calculate the start- and endrow for worker  
    int startrow = rank * ( matrix_properties[0] / num_worker);
    int endrow = ((rank + 1) * ( matrix_properties[0] / num_worker)) -1;
    
    /* calculate sub matrices */
    int number_of_rows = size_res / num_worker;
    double *result_matrix = calloc(number_of_rows, sizeof(double));

    int position = 0;

    for (int i = startrow; i <= endrow; i++) {
        for (int j = 0; j < matrix_properties[3]; j++) {
            for (int k = 0; k < matrix_properties[2]; k++) {
                result_matrix[position] +=
                    m_a[ (i * matrix_properties[1] + k) ] *
                    m_b[ (k * matrix_properties[3] + j) ];
            }
            position++;
        }
    }
    
    free(m_a);
    free(m_b);
    
    /* collect the results */
    MPI_Gather(result_matrix, number_of_rows, MPI_DOUBLE,
           final_matrix, number_of_rows,  MPI_DOUBLE, 0, MPI_COMM_WORLD);

    /** The master presents the results on the console */
    if (rank == 0){
        int size = matrix_properties[0] * matrix_properties[3];
        int i = 0;
        while (i < size) {
            printf("%lf\t", final_matrix[i]);
            i++;
        
            if (i % matrix_properties[3] == 0)
                printf("\n");
        }
    }
    
    free(result_matrix);
    free(final_matrix);
    
    MPI_Finalize();
    exit(EXIT_SUCCESS);
}
