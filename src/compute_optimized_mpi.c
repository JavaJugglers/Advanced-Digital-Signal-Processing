#include <omp.h>
#include <x86intrin.h>
#include "compute.h"

int flippings(matrix_t *b_matrix) {
    uint32_t rows = b_matrix->rows;
    uint32_t cols = b_matrix->cols;
    uint32_t colsMinusOne = cols - 1;
    uint32_t rowsMinusOne = rows - 1;

    #pragma omp parallel for
    for (uint32_t i = 0; i < rows; i++) {
        for (uint32_t j = 0; j < cols / 2; j++) {
            uint32_t flipped_j = colsMinusOne - j;
            uint32_t temp = b_matrix->data[i * cols + j];
            b_matrix->data[i * cols + j] = b_matrix->data[i * cols + flipped_j];
            b_matrix->data[i * cols + flipped_j] = temp;
        }
    }

    #pragma omp parallel for
    for (uint32_t i = 0; i < rows / 2; i++) {
        for (uint32_t j = 0; j < cols; j++) {
            uint32_t flipped_i = rowsMinusOne - i;
            uint32_t temp = b_matrix->data[i * cols + j];
            b_matrix->data[i * cols + j] = b_matrix->data[flipped_i * cols + j];
            b_matrix->data[flipped_i * cols + j] = temp;
        }
    }
    return 0;
}

int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
    flippings(b_matrix);

    uint32_t b_rows = b_matrix->rows;
    uint32_t b_cols = b_matrix->cols;

    uint32_t a_rows = a_matrix->rows;
    uint32_t a_cols = a_matrix->cols;

    uint32_t out_rows = a_matrix->rows - b_matrix->rows + 1;
    uint32_t out_cols = a_matrix->cols - b_matrix->cols + 1;

    *output_matrix = (matrix_t *)malloc(sizeof(matrix_t));

    (*output_matrix)->rows = out_rows;
    (*output_matrix)->cols = out_cols;


    (*output_matrix)->data = (int32_t *)malloc(out_rows * out_cols * sizeof(int32_t));
    int32_t sum = 0;
   #pragma omp parallel for collapse(2) schedule(dynamic) reduction(+:sum)
    for (uint32_t a_row = 0; a_row < out_rows; ++a_row) {
        for (uint32_t a_col = 0; a_col < out_cols; ++a_col) {
            sum = 0;
            for (uint32_t b_row = 0; b_row < b_rows; ++b_row) {
                uint32_t a_index_base = (a_row + b_row) * a_cols + a_col;
                uint32_t b_index_base = b_row * b_cols;
                uint32_t b_col = 0;
                if (b_cols >= 24) {
    for (; b_col <= b_cols - 24; b_col += 24) {
        // Load first 8 elements
        __m256i result_1 = _mm256_mullo_epi32(_mm256_loadu_si256((__m256i*)&a_matrix->data[a_index_base + b_col]), _mm256_loadu_si256((__m256i*)&b_matrix->data[b_index_base + b_col]));


        __m256i result_2 = _mm256_mullo_epi32(_mm256_loadu_si256((__m256i*)&a_matrix->data[a_index_base + b_col + 8]), _mm256_loadu_si256((__m256i*)&b_matrix->data[b_index_base + b_col + 8]));


        __m256i result_3 = _mm256_mullo_epi32(_mm256_loadu_si256((__m256i*)&a_matrix->data[a_index_base + b_col + 16]), _mm256_loadu_si256((__m256i*)&b_matrix->data[b_index_base + b_col + 16]));

        // Horizontal addition for first 8 elements
        result_1 = _mm256_hadd_epi32(result_1, result_1);
        result_1 = _mm256_hadd_epi32(result_1, result_1);


        result_2 = _mm256_hadd_epi32(result_2, result_2);
        result_2 = _mm256_hadd_epi32(result_2, result_2);


        result_3 = _mm256_hadd_epi32(result_3, result_3);
        result_3 = _mm256_hadd_epi32(result_3, result_3);

        // Summing
        sum += _mm_cvtsi128_si32(_mm_add_epi32(_mm256_castsi256_si128(result_1), _mm256_extracti128_si256(result_1, 1))) + _mm_cvtsi128_si32(_mm_add_epi32(_mm256_castsi256_si128(result_2), _mm256_extracti128_si256(result_2, 1))) + _mm_cvtsi128_si32(_mm_add_epi32(_mm256_castsi256_si128(result_3), _mm256_extracti128_si256(result_3, 1)));
    }
}
                if (b_cols >= 8) {
                    for (; b_col <= b_cols - 8; b_col += 8) {
                        __m256i result = _mm256_mullo_epi32(_mm256_loadu_si256((__m256i*)&a_matrix->data[a_index_base + b_col]), _mm256_loadu_si256((__m256i*)&b_matrix->data[b_index_base + b_col]));

                        result = _mm256_hadd_epi32(result, result);
                        result = _mm256_hadd_epi32(result, result);
                        sum += _mm_cvtsi128_si32(_mm_add_epi32( _mm256_castsi256_si128(result), _mm256_extracti128_si256(result, 1)));
                    }
                }
                for (; b_col < b_cols; ++b_col) {
                    sum += a_matrix->data[a_index_base + b_col] * b_matrix->data[b_index_base + b_col];
                }
            }
            #pragma omp critical
            {
                (*output_matrix)->data[a_row * out_cols + a_col] = sum;
            }
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