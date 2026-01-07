#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mmio.h"
#ifndef WEAK_SCALING
#define WEAK_SCALING 0
#endif


typedef struct {
    int i, j;     
    double v;
} COO;




 int cmp_COO(const void *a, const void *b) {
    const COO *x = (const COO*)a;
    const COO *y = (const COO*)b;
    if (x->i != y->i) return (x->i < y->i) ? -1 : 1;
    if (x->j != y->j) return (x->j < y->j) ? -1 : 1;
    return 0;
}


 COO *syntheticMatrix(int size,int base_rows, int base_cols, int nnz_per_row, unsigned seed, int *ROW, int *COL, int *nz){
    *ROW = base_rows * size;
    *COL = base_cols * size;
    if (nnz_per_row > *COL) nnz_per_row = *COL;
    *nz  = (*ROW) * nnz_per_row;

    COO *T = (COO*)malloc((size_t)(*nz) * sizeof(COO));
    if (!T) return NULL;

    srand(seed);

    int *used = (int*)malloc((size_t)nnz_per_row * sizeof(int));
    if (!used) { free(T); return NULL; }

    int idx = 0;
    for (int i = 0; i < *ROW; i++) {
        int filled = 0;
        while (filled < nnz_per_row) {
            int col = rand() % (*COL);

            int dup = 0;
            for (int t = 0; t < filled; t++) {
                if (used[t] == col) { dup = 1; break; }
            }
            if (dup) continue;

            used[filled++] = col;
            T[idx].i = i;
            T[idx].j = col;
            T[idx].v = (double)rand() / (double)RAND_MAX;
            idx++;
        }
    }

    free(used);

    return T;
}


COO *read_realmatrices(const char *path, int *ROW, int *COL, int *nz) {
    MM_typecode matcode;
    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    if (mm_read_banner(f, &matcode) != 0) { fclose(f); return NULL; }
    if (mm_is_complex(matcode) && mm_is_matrix(matcode) && mm_is_sparse(matcode)) {
        fclose(f); return NULL;
    }
    if (mm_read_mtx_crd_size(f, ROW, COL, nz) != 0) { fclose(f); return NULL; }

    COO *T = (COO*)malloc((size_t)(*nz) * sizeof(COO));
    if (!T) { fclose(f); return NULL; }

    for (int k = 0; k < *nz; k++) {
        int I, J;
        double V;
        if (fscanf(f, "%d %d %lg\n", &I, &J, &V) != 3) { free(T); fclose(f); return NULL; }
        T[k].i = I - 1;
        T[k].j = J - 1;
        T[k].v = V;
    }
    fclose(f);

  qsort(T, (size_t)(*nz), sizeof(COO), cmp_COO);
    return T;
}
COO *load_or_generate_matrix(MPI_Comm comm, int rank, int size, const char *mtx_path,int base_rows, int base_cols, int nnz_per_row, unsigned seed, int *ROW, int *COL, int *nz){
    COO *T = NULL;

#if WEAK_SCALING
    if (rank == 0) {
        T = syntheticMatrix(size, base_rows, base_cols, nnz_per_row, seed, ROW, COL, nz);
        if (!T) { fprintf(stderr, "Error generating random COO\n"); MPI_Abort(comm, 10); }
    }
#else
    if (rank == 0) {
        if (!mtx_path) { fprintf(stderr, "Missing matrix.mtx\n"); MPI_Abort(comm, 11); }
        T = read_realmatrices(mtx_path, ROW, COL, nz);
        if (!T) { fprintf(stderr, "Error reading MatrixMarket\n"); MPI_Abort(comm, 12); }
    }
#endif

    MPI_Bcast(ROW, 1, MPI_INT, 0, comm);
    MPI_Bcast(COL, 1, MPI_INT, 0, comm);
    MPI_Bcast(nz,  1, MPI_INT, 0, comm);

    return T;
}


 MPI_Datatype COO_type(void) {
    MPI_Datatype t;
    COO dummy;

    int blocklen[3] = {1, 1, 1};
    MPI_Aint disp[3], base;
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_DOUBLE};

    MPI_Get_address(&dummy,   &base);
    MPI_Get_address(&dummy.i, &disp[0]);
    MPI_Get_address(&dummy.j, &disp[1]);
    MPI_Get_address(&dummy.v, &disp[2]);

    disp[0] -= base;
    disp[1] -= base;
    disp[2] -= base;

    MPI_Type_create_struct(3, blocklen, disp, types, &t);
    MPI_Type_commit(&t);
    return t;
}


 int randomNumbers(int max, int min) {
    return rand() % (max - min + 1) + min;
}

 void initvec(int *x, int n) {
    for (int i = 0; i < n; i++) x[i] = randomNumbers(1000, 1);
}





 void row_block(int ROW, int size, int rank, int *row0, int *nrows) {
    int base = ROW / size;
    int rem  = ROW % size;
    *nrows = base + (rank < rem ? 1 : 0);
    *row0  = rank * base + (rank < rem ? rank : rem);
}


 void initcsr(int *csr, const int *arow_local, int count, int nrow_local) {
    for (int i = 0; i <= nrow_local; i++) csr[i] = 0;
    for (int k = 0; k < count; k++) csr[arow_local[k] + 1]++;      
    for (int i = 0; i < nrow_local; i++) csr[i + 1] += csr[i];
}


 void multiply(double *y, const double *aval, const int *acol, const int *csr, const int *x, int nrow_local) {
    

	for (int i = 0; i < nrow_local; i++) {
        double sum = 0.0;
        for (int k = csr[i]; k < csr[i + 1]; k++){
			sum += aval[k] * (double)x[acol[k]];
		}	
        y[i] = sum;
    }
}

int main(int argc, char **argv) {
    
	MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
#if !WEAK_SCALING
    if (argc < 2) {
        if (rank == 0) fprintf(stderr, "Usage: %s matrix.mtx\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
#endif

    int ROW = 0, COL = 0, nz = 0;
    COO *T = NULL;              // only rank 0 
    int *nnz_counts = NULL;         // only rank 0 used 
    int *nnz_start = NULL;         // only rank 0 used 

   
	
    const char *mtx_path = (argc >= 2 ? argv[1] : NULL);

    // synthetic (WEAK_SCALING=1) 
    int base_rows = 10000;
    int base_cols = 10000;
    int nnz_per_row = 50;
    unsigned seed = (unsigned)time(NULL);

    T = load_or_generate_matrix(MPI_COMM_WORLD, rank, size, mtx_path, base_rows, base_cols, nnz_per_row, seed, &ROW, &COL, &nz);


  
    int row0, nrows_local;
    row_block(ROW, size, rank, &row0, &nrows_local);

    // vector
    int *x = (int*)malloc((size_t)COL * sizeof(int));
    if (rank == 0) { 
	srand((unsigned)time(NULL)); initvec(x, COL); 
	}
    MPI_Bcast(x, COL, MPI_INT, 0, MPI_COMM_WORLD);

   
    MPI_Datatype MPI_COO = COO_type();

    
    int nnz_local = 0;
    if (rank == 0) {
        nnz_counts = (int*)calloc((size_t)size, sizeof(int));
        nnz_start = (int*)malloc((size_t)size * sizeof(int));

        int r = 0;
        int r0, rn;

        for (int k = 0; k < nz; k++) {
            int row = T[k].i;

            while (r + 1 < size) {
                row_block(ROW, size, r, &r0, &rn);
                if (row < r0 + rn) break;
                r++;
            }
            nnz_counts[r]++;
        }

        nnz_start[0] = 0;
        for (int i = 1; i < size; i++) nnz_start[i] = nnz_start[i - 1] + nnz_counts[i - 1];
    }

    /* each rank receives its nnz_local */
    MPI_Scatter(nnz_counts, 1, MPI_INT, &nnz_local, 1, MPI_INT, 0, MPI_COMM_WORLD);

    COO *Tloc = (COO*)malloc((size_t)(nnz_local > 0 ? nnz_local : 1) * sizeof(COO));

    
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = 0.0, t1 = 0.0;
    if (rank == 0) t0 = MPI_Wtime();

    
    MPI_Scatterv(T, nnz_counts, nnz_start, MPI_COO, Tloc, nnz_local, MPI_COO, 0, MPI_COMM_WORLD);

    // build local CSR from Tloc
    int *arow = (int*)malloc((size_t)(nnz_local > 0 ? nnz_local : 1) * sizeof(int));
    int *acol = (int*)malloc((size_t)(nnz_local > 0 ? nnz_local : 1) * sizeof(int));
    double *aval = (double*)malloc((size_t)(nnz_local > 0 ? nnz_local : 1) * sizeof(double));

    for (int k = 0; k < nnz_local; k++) {
        arow[k] = Tloc[k].i - row0;   // local row 
        acol[k] = Tloc[k].j;
        aval[k] = Tloc[k].v;
         
           if (arow[k] < 0 || arow[k] >= nrows_local) { printf("bad row\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
        
    }
 
    int *csr = (int*)malloc((size_t)(nrows_local + 1) * sizeof(int));
    initcsr(csr, arow, nnz_local, nrows_local);

 
    double *y_local = (double*)malloc((size_t)(nrows_local > 0 ? nrows_local : 1) * sizeof(double));
   
    MPI_Barrier(MPI_COMM_WORLD);
    double tmul0 = MPI_Wtime();
    multiply(y_local, aval, acol, csr, x, nrows_local);
	double tmul1 = MPI_Wtime();
	double t_mul = tmul1 - tmul0;
	
	double t_spmv = 0.0;
	MPI_Reduce(&t_mul, &t_spmv, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
    
    double *y = NULL;
    int *y_counts = NULL, *y_start = NULL;

    if (rank == 0) {
        y = (double*)malloc((size_t)ROW * sizeof(double));
        y_counts = (int*)malloc((size_t)size * sizeof(int));
        y_start = (int*)malloc((size_t)size * sizeof(int));

        for (int k = 0; k < size; k++) {
            int rr0, rrn;
            row_block(ROW, size, k, &rr0, &rrn);
            y_counts[k] = rrn;
        }
        y_start[0] = 0;
        for (int k = 1; k < size; k++) y_start[k] = y_start[k - 1] + y_counts[k - 1];
    }
	
    MPI_Gatherv(y_local, nrows_local, MPI_DOUBLE, y, y_counts, y_start, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		
    if (rank == 0) {
        t1 = MPI_Wtime();
       	
       	 double time = t1 - t0;
    	double flops = 2*nz;
    	


    	printf("Time RTT: %f s\n", time);
    	printf("Time SpMV max per Rank: %f s\n", t_spmv);
    	printf("FLOPs: %f\n", flops);
    	printf("nz %d rows %d cols %d\n", nz,ROW,COL);
       	
       	 
      
    }
	int nnz_min, nnz_max, nnz_sum;
	MPI_Reduce(&nnz_local, &nnz_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&nnz_local, &nnz_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&nnz_local, &nnz_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank==0){
  		double avg = (double)nnz_sum / size;
  		printf("nnz per rank: min=%d max=%d avg=%.1f (max/avg=%.2f)\n", nnz_min, nnz_max, avg, nnz_max/avg);
}



    free(x);
    free(Tloc);
    free(arow);
    free(acol);
    free(aval);
    free(csr);
    free(y_local);

    if (rank == 0) {
        free(T);
        free(nnz_counts);
        free(nnz_start);
        free(y);
        free(y_counts);
        free(y_start);
    }

    MPI_Type_free(&MPI_COO);
    MPI_Finalize();
    return 0;
}

