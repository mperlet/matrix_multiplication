#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "matrix.h"
#include <pthread.h>

#define DEFAULT_NUM_THREADS 4

struct v {
    int i; /* row */
    int j; /* column */
    matrix_struct *m_1;
    matrix_struct *m_2;
    matrix_struct *result_matrix;
};

void *runner(void *param); /* the thread */

int main(int argc, char **argv)
{
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

    
    int i,j, count = 0;
    for(i = 0; i < result_matrix->rows; i++) {
        
        for(j = 0; j < result_matrix->cols; j++) {
            //Assign a row and column for each thread
            struct v *data = (struct v *) malloc(sizeof(struct v));
            data->i = i;
            data->j = j;
            data->m_1 = m_1;
            data->m_2 = m_2;
            data->result_matrix = result_matrix;

            /* Now create the thread passing it data as a parameter */
            pthread_t tid;       //Thread ID
            pthread_attr_t attr; //Set of thread attributes
            //Get the default attributes
            pthread_attr_init(&attr);
            //Create the thread
            pthread_create(&tid,&attr,runner,data);
            //Make sure the parent waits for all thread to complete
            pthread_join(tid, NULL);
            count++;
        }
    }
   

    print_matrix(result_matrix);
    
    free_matrix(m_1);
    free_matrix(m_2);
    free_matrix(result_matrix);
    
    exit(EXIT_SUCCESS);
}

//The thread will begin control in this function
void *runner(void *param) {
    struct v *data = param; // the structure that holds our data
    int n = 0; //the counter and sum
    double sum = 0.0;
   
    //Row multiplied by column
    for(n = 0; n < data->m_2->rows; n++){
        sum += data->m_1->mat_data[data->i][n] * data->m_2->mat_data[n][data->j];
    }
   
    //assign the sum to its coordinate
    data->result_matrix->mat_data[data->i][data->j] += sum; 

    //Exit the thread
    pthread_exit(0);
}
