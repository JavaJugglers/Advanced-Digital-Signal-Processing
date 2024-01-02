#include "compute.h"
void flip_verticalss(matrix_t *matrix) {
    uint32_t total_rows = matrix->rows;

    for (uint32_t row = 0; row < total_rows / 2; ++row) {
        for (uint32_t col = 0; col < matrix->cols; ++col) {
            // Get first index
            uint32_t index_1 = (row * matrix->cols) + col;

            // Calculate the corresponding index in the opposite row
            uint32_t opposite_row = total_rows - 1 - row;
            uint32_t idx2 = (opposite_row * matrix->cols) + col;

            // Swap elements
            int32_t temp = matrix->data[index_1];
            matrix->data[index_1] = matrix->data[idx2];
            matrix->data[idx2] = temp;
        }
    }
}
void flip_horizontalss(matrix_t *matrix) {
    if (matrix == NULL || matrix->data == NULL) {
        return;
    }

    uint32_t total_columns = matrix->cols;
    uint32_t half_columns = total_columns / 2;

    for (uint32_t row = 0; row < matrix->rows; ++row) {
        for (uint32_t col = 0; col < half_columns; ++col) {
            uint32_t index1 = row * total_columns + col;
            uint32_t index2 = row * total_columns + (total_columns - col - 1);
            int32_t temp = matrix->data[index1];
            matrix->data[index1] = matrix->data[index2];
            matrix->data[index2] = temp;
        }
    }
}

int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
    flip_verticalss(b_matrix);
    flip_horizontalss(b_matrix);
    uint32_t out_rows = a_matrix->rows - b_matrix->rows + 1;
    uint32_t out_cols = a_matrix->cols - b_matrix->cols + 1;

    *output_matrix = (matrix_t *)malloc(sizeof(matrix_t));
    if (*output_matrix == NULL) {
        return -1;
    }

    (*output_matrix)->rows = out_rows;
    (*output_matrix)->cols = out_cols;
    (*output_matrix)->data = (int32_t *)malloc(out_rows * out_cols * sizeof(int32_t));
    if ((*output_matrix)->data == NULL) {
        free(*output_matrix);
        *output_matrix = NULL; 
        return -1;
    }

    for (uint32_t a_row = 0; a_row < out_rows; ++a_row) {
        for (uint32_t a_col = 0; a_col < out_cols; ++a_col) {
            int32_t sum = 0;
            for (uint32_t b_row = 0; b_row < b_matrix->rows; ++b_row) {
                for (uint32_t b_col = 0; b_col < b_matrix->cols; ++b_col) {
                    sum += a_matrix->data[(a_row + b_row) * a_matrix->cols + (a_col + b_col)] * b_matrix->data[b_row * b_matrix->cols + b_col];
                }
            }
            (*output_matrix)->data[a_row * out_cols + a_col] = sum;
        }
    }
    return 0;
}

// Executes a task
int execute_task(task_t *task) {
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  char *a_matrix_path = get_a_matrix_path(task);
  if (read_matrix(a_matrix_path, &a_matrix)) {
    printf("Error reading matrix from %s\n", a_matrix_path);
    return -1;
  }
  free(a_matrix_path);

  char *b_matrix_path = get_b_matrix_path(task);
  if (read_matrix(b_matrix_path, &b_matrix)) {
    printf("Error reading matrix from %s\n", b_matrix_path);
    return -1;
  }
  free(b_matrix_path);

  if (convolve(a_matrix, b_matrix, &output_matrix)) {
    printf("convolve returned a non-zero integer\n");
    return -1;
  }

  char *output_matrix_path = get_output_matrix_path(task);
  if (write_matrix(output_matrix_path, output_matrix)) {
    printf("Error writing matrix to %s\n", output_matrix_path);
    return -1;
  }
  free(output_matrix_path);

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}
