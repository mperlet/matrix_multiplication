typedef struct {
        unsigned int rows;
        unsigned int cols;
        double **mat_data;
} matrix_struct;

matrix_struct *get_matrix_struct(char matrix[]);
void print_matrix(matrix_struct *matrix_to_print);
void free_matrix(matrix_struct *matrix_to_free);

