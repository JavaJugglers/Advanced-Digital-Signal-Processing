# Video Processing Application Using Convolutions

## Project Overview

This project focuses on applying convolutions to video processing. Convolutions can blur, sharpen, or apply other effects to videos. By treating video frames as matrices of pixel values, we can perform various matrix operations, including convolution. This project simplifies the process by working with grayscale videos, allowing us to use single-value pixels.

### Key Concepts:

- **Convolutions:** Altering video frames by averaging pixel values to create effects like blurring (Gaussian Blur).
- **Vectors and Matrices:** Key data structures for handling video frames and convolution operations.
- **Binary Files:** Method of storing matrices for use in convolution.

### Components:

1. **Matrix Representation:**
   ```c
   typedef struct {
     uint32_t rows;
     uint32_t cols;
     int32_t *data;
   } matrix_t;
   ```
   Matrices are stored in `.bin` files in row-major order.

2. **Tasks:**
   ```c
   typedef struct {
     char *path;
   } task_t;
   ```
   Each task represents a convolution operation, identified by its path.

3. **Testing Framework:**
   - Tests located in the `tests` directory.
   - Custom tests can be added to `tools/custom_tests.py`.

4. **Directory Structure:**
   ```
   61c-proj4/
   ├── src/
   ├── tests/
   │   └── my_custom_test/
   │       ├── input.txt
   │       └── task0/
   │           ├── a.bin
   │           └── ...
   ├── tools/
   └── Makefile
   ```

5. **Testing and Debugging:**
   - Use the `Makefile` for running tests.
   - Tools like `cgdb` and `valgrind` are recommended for debugging.

### Convolution Mechanics

Convolution involves overlapping two matrices or vectors (matrix A and B) and computing the weighted average of overlapped elements. The process includes flipping matrix B and sliding it across matrix A, performing element-wise multiplication at each step.

### Implementation Details

1. **Naive Implementation:**
   - Implement `convolve` in `compute_naive.c`.
   - Arguments: `matrix_t* a_matrix`, `matrix_t* b_matrix`, `matrix_t** output_matrix`.
   - Return value: `int` (0 for success, -1 for errors).

2. **Optimization:**
   - Use SIMD instructions in `compute_optimized.c`.
   - Employ OpenMP directives for further speedup.

### Benchmarks and Performance

- Speedup requirements and benchmarks are provided.
- Performance is measured based on execution time and speedup over various test scenarios.

## Speedup Requirements

The following table outlines the speedup requirements for various benchmarks:

| Name        | Folder Name             | Type      | Speedup |
|-------------|-------------------------|-----------|---------|
| Random      | test_ag_random          | Optimized | 7.83x   |
|             |                         | Open MPI  | 5.19x   |
| Increasing  | test_ag_increasing      | Optimized | 9.03x   |
|             |                         | Open MPI  | 4.01x   |
| Decreasing  | test_ag_decreasing      | Optimized | 8.69x   |
|             |                         | Open MPI  | 4.62x   |
| Big and Small| test_ag_big_and_small  | Open MPI  | 3.07x   |

These benchmarks are critical in evaluating the performance enhancements achieved through optimizations in convolution operations.

