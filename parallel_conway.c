// For alabama super computer
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>

/***********************************************************************************
 *  Name: Caleb J. Messerly                                                         *
 *  Email: cjmesserly@crimson.ua.edu                                                *
 *  Course: CS 581                                                                  *
 *  Homework #: 4                                                                   *
 *  To compile, use the make file. To make with the icx compiler, do make icx.
 *  To compile with gcc, use make gcc.
 *  If you would prefer to compile manually, use the following compile line
 *  for icx and gcc, respectively:
 *	mpicc -g -Wall -o conway parallel_conway.c

 *  Example Run for 5000x5000 board with 5000 generations, 1 thread to scratch/test.txt:
 *  ./conway 5000 5000 1 scratch/test.txt					     *
 ************************************************************************************/

// Struct to get time
#define GET_TIME(now)                       \
  {                                         \
    struct timeval t;                       \
    gettimeofday(&t, NULL);                 \
    now = t.tv_sec + t.tv_usec / 1000000.0; \
  }

// Swaps map1 and map 2
void swap(short int ***a, short int ***b)
{
  short int **tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

// Prints array for debugging purposes
// Very simple
void printarray(short int **a, int mrows, int ncols)
{
  int i, j;

  for (i = 1; i < mrows - 1; i++)
  {
    for (j = 1; j < ncols - 1; j++)
      if (a[i][j] == 1)
      {
        printf("X ");
      }
      else
      {
        printf("O ");
      }

    printf("\n");
  }
}

int conway(short int **map1, short int **map2, int nCols, int nRows, int comm_sz, int local_start, int local_end, int my_rank, int partition)
{
  MPI_Barrier(MPI_COMM_WORLD);
  // Later used for determining the range iterated on
  int upper_val = -1;
  int lower_val = -1;

  // If only one thread is used, skip pulling neighboring partitions
  if (comm_sz > 1)
  {

    // Send my map's value up the chain (IE, Thread 1 sends up to 2)
    if (my_rank != comm_sz - 1 && my_rank != 0)
    {
      MPI_Sendrecv(&(map1[local_start][0]), partition, MPI_SHORT, (my_rank + 1), 0, &map1[((my_rank - 1) * partition / nCols + 1)][0], partition, MPI_SHORT, (my_rank - 1), 0, MPI_COMM_WORLD, NULL);
    }
    else
    {
      // If the first/last one, they should only send/recv, respectively
      if (my_rank == comm_sz - 1)

        MPI_Recv(&map1[((my_rank - 1) * partition / nCols + 1)][0], partition, MPI_SHORT, comm_sz - 2, 0, MPI_COMM_WORLD, NULL);
      else // Process 0
        MPI_Send(&(map1[local_start][0]), partition, MPI_SHORT, (my_rank + 1), 0, MPI_COMM_WORLD);
    }

    // Send my map's value up the chain (IE, Thread 2 sends down to 1)
    if (my_rank != comm_sz - 1 && my_rank != 0)
    {
      MPI_Sendrecv(&(map1[local_start][0]), partition, MPI_SHORT, (my_rank - 1), 0, &map1[((my_rank + 1) * partition / nCols + 1)][0], partition, MPI_SHORT, (my_rank + 1), 0, MPI_COMM_WORLD, NULL);
    }
    else
    {
      if (my_rank == comm_sz - 1)
        MPI_Send(&(map1[local_start][0]), partition, MPI_SHORT, comm_sz - 2, 0, MPI_COMM_WORLD);
      else // Process 0
        MPI_Recv(&map1[((my_rank + 1) * partition / nCols + 1)][0], partition, MPI_SHORT, 1, 0, MPI_COMM_WORLD, NULL);
    }
  }

  // Declares sentinel change value
  int changes = 0;
  MPI_Barrier(MPI_COMM_WORLD);
  // Iterates over all values in map
  // Notice the for loop has i_start and i_end as the iterator, this ensures the threads all modify a different part of the map
  for (int i = local_start; i < local_end; i++)
  {
    for (int j = 1; j < nCols - 1; j++)
    {
      // This value will hold the number of neighbors
      int neighborCount = 0;
      // New value in spot
      short int newValue = 0;

      // Get number of alive neighbors
      neighborCount = map1[i - 1][j + 1] + map1[i][j + 1] + map1[i + 1][j + 1] + map1[i - 1][j] + map1[i + 1][j] + map1[i - 1][j - 1] + map1[i][j - 1] + map1[i + 1][j - 1]; // printf("TOTAL: %i\n", total);
      // If 2 or 3 alive neighbors, new value is 1. Else, it remains 0
      if (neighborCount == 3)
      {
        newValue = 1;
      }
      else if (neighborCount == 2)
      {
        newValue = map1[i][j];
      }
      else
      {
        newValue = 0;
      }
      // Assign new value to next matrix
      map2[i][j] = newValue;
      // Tracking if a change has occured
      if (map2[i][j] != map1[i][j])
      {
        changes++;
      }
    }
  }

  return changes;
}

int main(int argc, char **argv)
{

  // Checks cmd args
  if (argc != 4)
  {
    printf("Usage: ./<executable(conway)> <board-size> <num-generations> <output_directory>\n");
    return 0;
  }

  // Declares timing variables for later
  double starttime, endtime;
  // Assigns variables from cmdline args
  // Adds 2 to rows and cols for dead border
  int nRows = atoi(argv[1]) + 2;
  int nCols = nRows;
  int numGenerations = atoi(argv[2]);
  char *output_directory = argv[3];
  int my_rank, comm_sz;
  short int *block1, *block2;
  short int **map1, **map2;
  int terminated_gen = 0;
  int special_partition_size;

  // Initialize MPI
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

  // Declares arrays in row major storage
  map1 = (short int **)malloc(nCols * sizeof(short int *));
  block1 = (short int *)malloc(nRows * nCols * sizeof(short int));
  map2 = (short int **)malloc(nCols * sizeof(short int *));
  block2 = (short int *)malloc(nRows * nCols * sizeof(short int));

  // Setting up row major storage
  for (int i = 0; i < nRows; i++)
  {
    map1[i] = &block1[i * nCols];
    map2[i] = &block2[i * nCols];
  }
  srand48(12345);

  if (my_rank == 0)
  {
    // Initial values
    for (int i = 0; i < nRows; i++)
    {
      for (int j = 0; j < nCols; j++)
      {
        if (i == 0 || i == nCols - 1 || j == 0 || j == nCols - 1)
        {
          map1[i][j] = 0;
          map2[i][j] = 0;
        }
        else
        {
          map1[i][j] = drand48() > 0.5 ? 1 : 0;
        }
      }
    }
    // printarray(map1, nRows, nCols);
  }
  else if (my_rank != 0)
  {
    for (int i = 0; i < nRows; i++)
    {
      for (int j = 0; j < nCols; j++)
      {
        map1[i][j] = 0;
        map2[i][j] = 0;
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // Sets the partition size
  int partition_size = ((nRows - 2) * nRows) / comm_sz;
  // Determines starting row based on thread number. Divide by nCols to put in row units
  int local_start = my_rank * partition_size / nCols + 1;
  // Determines ending row based on locat start
  int local_end = local_start + partition_size / nCols;

  // Special case for 16 threads
  if (comm_sz == 16)
  {
    partition_size = 312 * 5002;
    special_partition_size = 320 * 5002;
    // Determines starting row based on thread number. Divide by nCols to put in row units
    local_start = my_rank * partition_size / nCols + 1;
    // Determines ending row based on locat start
    local_end = local_start + partition_size / nCols;
  }

  if (my_rank == 0)
  {
    local_start = 1;
  }
  else if (my_rank == comm_sz - 1)
  {
    local_end = nRows - 1;
  }

  printf("Range: [%d - %d)\n", local_start, local_end);

  // Scatters starting map to different threads, has them receive in map 2 because of a quirk in mpi_scatter not allowing you to send and receive from the same  buffer
  MPI_Scatter(&(map1[1][0]), partition_size, MPI_SHORT, &(map2[local_start][0]), partition_size, MPI_SHORT, 0, MPI_COMM_WORLD);
  // Makes sure every thread receives the starting map
  MPI_Barrier(MPI_COMM_WORLD);

  if (comm_sz == 16)
  {
    if (my_rank == 0)
    {
      MPI_Send(&(map1[4681][0]), special_partition_size, MPI_SHORT, 15, 0, MPI_COMM_WORLD);
    }
    else if (my_rank == 15)
    {
      MPI_Recv(&map2[local_start][0], special_partition_size, MPI_SHORT, 0, 0, MPI_COMM_WORLD, NULL);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // Sets map1 to map2 value
  for (int i = local_start; i < local_end; i++)
  {
    for (int j = 0; j < nCols; j++)
    {
      map1[i][j] = map2[i][j];
    }
  }

  // Starting game of life
  int changes = 0;
  double start = MPI_Wtime();
  int final_gen = 0;
  for (int i = 0; i < numGenerations; i++)
  {
    changes = 0;
    // Game of life
    changes = conway(map1, map2, nCols, nRows, comm_sz, local_start, local_end, my_rank, partition_size);
    // Switch map pointers
    swap(&map1, &map2);

    // Ensure everyone finishes their generation before moving on
    MPI_Barrier(MPI_COMM_WORLD);
    // if(my_rank ==0){
    //   printf("%d\n", i);
    // }

    int total_changes = 0;
    MPI_Allreduce(&changes, &total_changes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (total_changes == 0)
    {
      printf("Exiting early(gen %d) due to no change.\n", i);
      final_gen = i;
      break;
    }
    final_gen = i;
    MPI_Barrier(MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  double finish = MPI_Wtime();

  // Recombine map
  if (my_rank == 0 && comm_sz > 1)
  {
    for (int i = 1; i < comm_sz; i++)
    {

      MPI_Status status;
      if (comm_sz == 16)
      {
        if (i == comm_sz - 1)
        {
          MPI_Recv(&(map1[4681][0]), special_partition_size, MPI_SHORT, i, 0, MPI_COMM_WORLD, &status);
        }
        else
        {
          MPI_Recv(&(map1[i * 312 + 1][0]), partition_size, MPI_SHORT, i, 0, MPI_COMM_WORLD, &status);
        }
        printf("Received from row: %d\n", (i * 312 + 1));
      }
      else
      {
        MPI_Recv(&(map1[i * partition_size / nCols + 1][0]), partition_size, MPI_SHORT, i, 0, MPI_COMM_WORLD, &status);
      }
      printf("Received starting from row %d\n", status.MPI_SOURCE);

      // printf("Receiving - %d from %d\n", i*partition_size / nCols + 1, i);
    }
  }
  else if (comm_sz > 1 && my_rank != 0)
  {

    printf("Thread %d: Sending partition starting with row %d\n", my_rank, local_start);
    if (comm_sz == 16)
    {
      if (my_rank != comm_sz - 1)
      {
        MPI_Send(&(map1[local_start][0]), partition_size, MPI_SHORT, 0, 0, MPI_COMM_WORLD);
      }
      else if (my_rank == comm_sz - 1)
      {
        MPI_Send(&(map1[local_start][0]), special_partition_size, MPI_SHORT, 0, 0, MPI_COMM_WORLD);
      }
    }
    else
    {
      MPI_Send(&(map1[local_start][0]), partition_size, MPI_SHORT, 0, 0, MPI_COMM_WORLD);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  if (my_rank == 0)
  {
    printf("Test details:\n %d by %d board\n", nRows, nCols);
    printf("%d generations\n", numGenerations);
    printf("Threads: %d\n", comm_sz);
    printf("Time taken = %lf seconds\n", (finish - start));
    printf("Terminated at generation %d\n", final_gen);
  }

  if (my_rank == 0)
  {

    // Writes results to the file
    FILE *file = fopen(output_directory, "w");

    if (file == NULL)
    {
      perror("Error opening file");
      return 1;
    }
    for (int i = 1; i < nRows - 1; i++)
    {
      printf("%d\n", i);

      for (int j = 1; j < nCols - 1; j++)
      {

        fprintf(file, "%d ", map1[i][j]);
      }
      fprintf(file, "\n"); // New line after each row
    }
    printf("Done\n");
    fclose(file);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // Free the memory
  free(block1);
  free(block2);
  free(map1);
  free(map2);

  MPI_Finalize();

  printf("Finished\n");

  return 0;
}
