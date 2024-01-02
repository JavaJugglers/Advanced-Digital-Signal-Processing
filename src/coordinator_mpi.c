#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coordinator.h"

#define READY 0
#define TERMINATE -1

int execute_task(task_t *task);
int read_tasks(char *input_file, int *num_tasks, task_t ***tasks);
char *get_a_matrix_path(task_t *task);
char *get_b_matrix_path(task_t *task);
char *get_output_matrix_path(task_t *task);
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Error: not enough arguments\n");
    printf("Usage: %s [path_to_task_list]\n", argv[0]);
    return -1;
  }
  int numTasks;
  task_t **tasks;
  if (read_tasks(argv[1], &numTasks, &tasks) != 0) {
    fprintf(stderr, "Failed to read tasks from file\n");
    return -1;
  }
  MPI_Init(&argc, &argv);
  int procID, totalProcs;
  MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &procID);

  if (procID == 0) {
    // Manager
    int nextTask = 0;
    MPI_Status status;
    int32_t message;
    while (nextTask < numTasks) {
      MPI_Recv(&message, 1, MPI_INT32_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      int sourceProc = status.MPI_SOURCE;
      message = nextTask;
      MPI_Send(&message, 1, MPI_INT32_T, sourceProc, 0, MPI_COMM_WORLD);
      nextTask++;
    }
    // Wait for process finish
    for (int i = 0; i < totalProcs - 1; i++) {
      MPI_Recv(&message, 1, MPI_INT32_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      int sourceProc = status.MPI_SOURCE;
      message = TERMINATE;
      MPI_Send(&message, 1, MPI_INT32_T, sourceProc, 0, MPI_COMM_WORLD);
    }
  } else {
    // Worker node
    int32_t message;
    while (true) {
      message = READY;
      MPI_Send(&message, 1, MPI_INT32_T, 0, 0, MPI_COMM_WORLD);
      MPI_Recv(&message, 1, MPI_INT32_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (message == TERMINATE) break;
      if (execute_task(tasks[message]) != 0) {
        fprintf(stderr, "Failed to execute task %d\n", message);
      }
    }
  }
  MPI_Finalize();
  return 0;
}

