# PARCO-Computing-2026-<242637>
# first deliverable: Sparce Matrix Vector Multiplication
Sparse matrix vector multiplication (SpMV)
 is one of the most computational kernel used in different
 f
 ields and analyzed more times to obtain the best possible
 performance solution.

# Clone the repository
open your local shell use
git clone https://github.com/tuouser/nomedelprogetto.git

# programming language
c++

# compiler version
g++ 9.1.0

 ## log in to HPC cluster

enter in a node with:

- qsub -I -q short_cpuQ -l select=1:ncpus=64:mem=64gb,walltime=01:00:00

load "first_deliverable_code" on the cluster and enter in this directory.
to compile the sequential code:
- vim main.cpp
go to the function multiply and comment:
  //   for (int s = 0; s < 3; s++) 
  //    for (int c = 0; c < 3; c++) 

//start= omp_get_wtime();
//      #pragma omp parallel for  num_threads(thread_count) /* simd*/  schedule(static, chunk_sizes[c])
//                 #pragma omp parallel for reduction(+:temp)
//stop = omp_get_wtime();
//      printf("Schedule: %s, Chunk Size: %d, Time: %f seconds\n", schedulee_types[s], chunk_sizes[c], stop - start);

decomment :  

GET_TIME(start);
GET_TIME(stop);
printf("time %f \n",stop-start);

and try to compile with:
- g++ -g  mmio.cpp main.cpp
ad run with:
- ./a.out matrix_name.mtx
for 1138_bus.mtx and adder_dcop_11.mtx you have to use the original mtx file, for the others you can use the sorted.mtx verison but you have to comment:       
// quicksortCOO(I, J, val, 0, nz - 1);
in the main function.
 
 to compile the parallel code without scheduling strategies:
 - vim main.cpp

 go to the function multiply and comment:
 // for (int s = 0; s < 3; s++) 
 // for (int c = 0; c < 3; c++) 
 // GET_TIME(start);
 // GET_TIME(stop);
 // printf("Schedule: %s, Chunk Size: %d, Time: %f seconds\n", schedulee_types[s], chunk_sizes[c], stop - start);
 

 here you have to comment only
 #pragma omp parallel for  num_threads(thread_count) /* simd  schedule(static, chunk_sizes[c]) */

decomment: 
 start= omp_get_wtime();
#pragma omp parallel for reduction(+:temp)
stop = omp_get_wtime();
 printf("time %f \n",stop-start);

 and compile with:
 - g++ -g -fopenmp mmio.cpp main.cpp

 and run with:
- ./a.out matrix_name.mtx thread_number
for 1138_bus.mtx and adder_dcop_11.mtx you have to use the original mtx file, for the others you can use the sorted.mtx verison but you have to comment:       
// quicksortCOO(I, J, val, 0, nz - 1);
in the main function.

to compile the parallel code with scheduling strategies but without simd vectorization:
 - vim main.cpp

 go to the function multiply and comment:

 // GET_TIME(start);
 // GET_TIME(stop);
 //printf("time %f \n",stop-start);

 here you have to comment only
 #pragma omp parallel for  num_threads(thread_count) /* simd */ schedule(static, chunk_sizes[c]) 

decomment: 
 for (int s = 0; s < 3; s++) 
 for (int c = 0; c < 3; c++) 
 start= omp_get_wtime();
#pragma omp parallel for reduction(+:temp)
stop = omp_get_wtime();
 printf("Schedule: %s, Chunk Size: %d, Time: %f seconds\n", schedulee_types[s], chunk_sizes[c], stop - start);


 and compile with:
 - g++ -g -fopenmp mmio.cpp main.cpp

 and run with:
- ./a.out matrix_name.mtx thread_number
for 1138_bus.mtx and adder_dcop_11.mtx you have to use the original mtx file, for the others you can use the sorted.mtx verison but you have to comment:       
// quicksortCOO(I, J, val, 0, nz - 1);
in the main function.

to compile the parallel code with scheduling strategies and simd vectorization:
 - vim main.cpp

 go to the function multiply and comment:

 // GET_TIME(start);
 // GET_TIME(stop);
 //printf("time %f \n",stop-start);

 here you have to comment only
 #pragma omp parallel for  /* num_threads(thread_count)*/  simd  schedule(static, chunk_sizes[c]) 

decomment: 
 for (int s = 0; s < 3; s++) 
 for (int c = 0; c < 3; c++) 
 start= omp_get_wtime();
#pragma omp parallel for reduction(+:temp)
stop = omp_get_wtime();
 printf("Schedule: %s, Chunk Size: %d, Time: %f seconds\n", schedulee_types[s], chunk_sizes[c], stop - start);


 and compile with:
 - g++ -O3 -march=native -fopenmp -ftree-vectorize  mmio.cpp main.cpp

 and run with:
- ./a.out matrix_name.mtx
for 1138_bus.mtx and adder_dcop_11.mtx you have to use the original mtx file, for the others you can use the sorted.mtx verison but you have to comment:       
// quicksortCOO(I, J, val, 0, nz - 1);
in the main function.

if you want to see the percentage of cache miss you have to write:
- module load perf
- perf stat -e cache-references,cache-misses ./a.out matrix_name.mtx 
if you have specified the number of threads when you have run the program:
- perf stat -e cache-references,cache-misses ./a.out matrix_name.mtx thread_number


