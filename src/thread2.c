#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "matrix.h"
#include <pthread.h>

struct v {
    int id; /* row */
    int last; // Flag for last loop round
    matrix_struct *m_1; // Matrix A
    matrix_struct *m_2; // Matrix B
    matrix_struct *result_matrix; // Result Matrix
};

void *worker(void *param); /* the thread */

int main(int argc, char **argv)
{
    int number_of_proc = sysconf(_SC_NPROCESSORS_ONLN);

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

    // allocate result matrix memory
    matrix_struct *result_matrix = malloc(sizeof(matrix_struct));
    result_matrix->rows = m_1->rows;
    result_matrix->cols = m_2->cols;
    result_matrix->mat_data = calloc(result_matrix->rows, sizeof(double*)); 
    for(int i=0; i < result_matrix->rows; ++i)
        result_matrix->mat_data[i]=calloc(result_matrix->cols, sizeof(double));


    // Allocate thread handles
    pthread_t *threads;
    threads = (pthread_t *) malloc(number_of_proc * sizeof(pthread_t));

    // thread creating loop
    for (int i = 0; i < number_of_proc; i++) {
        
        // fill the struct for thread worker
        struct v *data = (struct v *) malloc(sizeof(struct v));
        data->id = i;
        data->m_1 = m_1;
        data->m_2 = m_2;
        data->result_matrix = result_matrix;
        
        // set the flag if last thread
        if (i == number_of_proc-1)
            data->last = 1;
        
        else
            data->last = 0;
        
        // create thread
        pthread_create(&threads[i], NULL, worker, data);
    }

    // join all threads
    for (int i = 0; i < number_of_proc; i++) {
        pthread_join(threads[i], NULL);
    }

    print_matrix(result_matrix);
    
    free_matrix(m_1);
    free_matrix(m_2);
    free_matrix(result_matrix);
    
    exit(EXIT_SUCCESS);
}

void *worker(void *param) {
    
    // struct with matrix data
    struct v *data = param;
    
    // number of threads
    int number_of_proc = sysconf(_SC_NPROCESSORS_ONLN);

    // get thread id
    int thread_id = data->id;
    int count =     0;
    
    // get the parts of the matrix
    int thread_delta = data->result_matrix->rows/number_of_proc;
    
    int endrow;
    
    // compute the startrow
    int startrow = thread_id * thread_delta;
    
    // compute the last row (the last thread get the last rows)
    if(data->last != 1)
        endrow = startrow + thread_delta;
    else
        endrow = data->result_matrix->rows;

    // calculte the matrix part from startrow to endrow
    for (int i = startrow; i < endrow; i++) {
        for (int j = 0; j < data->result_matrix->cols; j++) {
            for (int k = 0; k < data->m_2->rows; k++) {
                data->result_matrix->mat_data[i][j] += data->m_1->mat_data[i][k] * data->m_2->mat_data[k][j];
                count++;
            }
        }
    }
    
    // debug info
    //printf("Thread-Nr: %d\t ThreadD: %d \tCalc#: %d \tStartRow: %d \tEndRow: %d \tLast: %d\n", thread_id,endrow-startrow, count, startrow, endrow,data->last);
    
    pthread_exit(0);
}

