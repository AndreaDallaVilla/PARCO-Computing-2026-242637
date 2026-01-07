# PARCO-Computing-2026-<242637>
# second deliverable: Distributed Sparce Matrix Vector Multiplication
Sparse matrix vector multiplication (SpMV)
 is one of the most computational kernel used in different
 f
 ields and analyzed more times to obtain the best possible
 performance solution.

# Clone the repository
open your local shell use
git clone https://github.com/AndreaDallaVilla/PARCO-Computing-2026-242637.git

# programming language
c

# compiler version
gcc 9.1.0

 ## log in to HPC cluster
you have to put the pbs into the home of HPC cluster.
modify mpi.pbs:
- vim mpi.pbs

For our experiment i use:
- #PBS -l select=1:ncpus=64:mpiprocs=1:mem=1mb
- #PBS -l select=1:ncpus=64:mpiprocs=4:mem=1mb
- #PBS -l select=1:ncpus=64:mpiprocs=8:mem=1mb
- #PBS -l select=1:ncpus=64:mpiprocs=16:mem=1mb
- #PBS -l select=1:ncpus=64:mpiprocs=32:mem=1mb
- #PBS -l select=1:ncpus=64:mpiprocs=64:mem=1mb
- #PBS -l select=2:ncpus=64:mpiprocs=64:mem=1mb
- #PBS -l select=4:ncpus=64:mpiprocs=64:mem=1mb

and below:
- mpirun -np 1 hostname
- mpirun -np 4 hostname
- mpirun -np 8 hostname
- mpirun -np 16 hostname
- mpirun -np 32 hostname
- mpirun -np 64 hostname
- mpirun -np 128 hostname
- mpirun -np 256 hostname

if you want to test the weak scaling you have to:
- mpicc -DWEAK_SCALING=1  main.c mmio.c -o code.out
and
- mpirun ./code.out
otherwise with strong scaling:
- mpicc main.c mmio.c -o code.out
and
-  mpirun ./code.out matrix.mtx