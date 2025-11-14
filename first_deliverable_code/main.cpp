
#include <iostream>
#include <time.h>
#include <cstdlib>
#include<string.h>
#include <omp.h>
#include "mmio.h"
#include "my_timer.h"



void initmatrix2(int *m,int row);
void initcsr( int *csr,int *arow, int count, int row);
void multiply(double *fm,double *aval,int *acol,int *csr,int *m2,int row,int thread_count );
void quicksortCOO(int *I, int *J, double *val, int first, int last);
int orderCOO(int *I, int *J, double *val, int first, int last);
void swapCOO(int *I, int *J, double *val, int a, int b);

int random(int max,int min);
using namespace std;


	 int M, N, nz;  
	 	
int main(int argc, char** argv) {
	srand(time(0));
	

	int thread_count = 1; 
	if (argc >= 3) thread_count = strtol(argv[2], NULL, 10);

 	double total_time = 0.0;

	int ret_code;
    MM_typecode matcode;
    FILE *f;
    int *m2;
    int i, *I, *J;
    double *val;
	int *csr;
	double *fm;
   
    if (argc < 2)
	{
		fprintf(stderr, "Usage: %s 1138_bus.mtx\n", argv[0]);
		exit(1);
	}
    else    
    { 
        if ((f = fopen(argv[1], "r")) == NULL) 
            exit(1);
    }

    if (mm_read_banner(f, &matcode) != 0)
    {
        printf("Could not process Matrix Market banner.\n");
        exit(1);
    }


    /*  This is how one can screen matrix types if their application */
    /*  only supports a subset of the Matrix Market data types.      */

    if (mm_is_complex(matcode) && mm_is_matrix(matcode) && 
            mm_is_sparse(matcode) )
    {
        printf("Sorry, this application does not support ");
        printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
        exit(1);
    }

    /* find out size of sparse matrix .... */

    if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) !=0)
        exit(1);


    /* reseve memory for matrices */

    I = (int *) malloc(nz * sizeof(int));
    J = (int *) malloc(nz * sizeof(int));
    val = (double *) malloc(nz * sizeof(double));
	csr = (int *) malloc((M+1) * sizeof(int));
 	m2 = (int *) malloc(N * sizeof(int));
 	fm = (double *) malloc(M * sizeof(double));
    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    for (i=0; i<nz; i++)
    {
        fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
        I[i]--;  /* adjust from 1-based to 0-based */
        J[i]--;
    }
/*quicksortCOO(I,J,val,0,nz-1);
f=fopen("sorted.txt","w");
for(i=0;i<nz;i++)
fprintf(f,"%d %d %20.19g\n",I[i],J[i],val[i]);*/
    if (f !=stdin) fclose(f);

    /************************/
    /* now write out matrix */
    /************************/

    mm_write_banner(stdout, matcode);
    mm_write_mtx_crd_size(stdout, M, N, nz);
    
	
	  /*  		
    
    for (i=0; i<nz; i++)
        fprintf(stdout, "%d %d %20.19g\n", I[i], J[i], val[i]);
        */
        
                                                                    	quicksortCOO(I, J, val, 0, nz - 1);
        	
	/*
		printf("\nrowI  colI  val\n");
        	for (i=0; i<nz; i++)
        printf("%d %d %20.19g\n", I[i], J[i], val[i]);
	
	*/
		initcsr(csr,I,nz,M);
		
		memset(fm, 0, M * sizeof(double));	
	
		
		
    
     /* 
		printf("\n csr\n");
	 for (i=0; i<=M; i++)
        printf("%d -> %d\n",i, csr[i]);
       */ 
			initmatrix2(m2,N);
/*
		printf("\n m2\n");
 	for (i=0; i<M; i++)
        printf("%d -> %d\n",i, m2[i]);
*/
	
	 
       
		multiply(fm,val,J,csr,m2,M,thread_count );
	 
    
	/*
		printf( "\n fm\n");
 	for (i=0; i<M; i++)
        printf("%d -> %20.19g\n",i, fm[i]);
	*/
	
free(fm);
free(m2);
free(csr);
free(val);
free(I);
free(J);		
	return 0;
}

int random(int max,int min){
	return rand()%(max-min+1)+min;
}


void quicksortCOO(int *I, int *J, double *val, int first, int last) {
    if (first < last) {
        int o = orderCOO(I, J, val, first, last);
        quicksortCOO(I, J, val, first, o - 1);
        quicksortCOO(I, J, val, o + 1, last);
    }
}

int orderCOO(int *I, int *J, double *val, int first, int last) {
    int Row = I[last];
    int Col = J[last];
    int i = first - 1;

    for (int j = first; j < last; j++) {
        if (I[j] < Row || (I[j] == Row && J[j] <= Col)) {
            i++;
            swapCOO(I, J, val, i, j);
        }
    }
    swapCOO(I, J, val, i + 1, last);
    return i + 1;
}

void swapCOO(int *I, int *J, double *val, int a, int b) {
    int tmpI = I[a];
    I[a] = I[b];
    I[b] = tmpI;

    int tmpJ = J[a];
    J[a] = J[b];
    J[b] = tmpJ;

    double tmpV = val[a];
    val[a] = val[b];
    val[b] = tmpV;

}
void initmatrix2(int *m,int row){

	
	for(int i=0;i<row;i++){
		
			
			m[i]=random(1000,1);
			
	
	}
	
}

void initcsr( int *csr,int *arow, int count, int row){
	

	for(int i=0; i<=row;i++)
	csr[i]=0;
	
	
	for(int i=0; i<count;i++)
	csr[arow[i]+1]++;

	for(int i=0; i<row;i++){
	
		csr[i+1]+=csr[i];

}
	

	
	
}

void multiply(double *fm,double *aval,int *acol,int *csr,int *m2,int row,int thread_count ){

	int chunk_sizes[] = {10, 100, 1000};
    char*  schedule_types[] = {"static", "dynamic", "guided"};

	for (int s = 0; s < 3; s++) {
      for (int c = 0; c < 3; c++) {
		

		 double start;
		 start= omp_get_wtime();
//		GET_TIME(start);
	#pragma omp parallel for  num_threads(thread_count) /* simd*/  schedule(static, chunk_sizes[c])
		for(int i=0;i<row;i++){
				 double temp= 0.0;
		   #pragma omp parallel for reduction(+:temp)
				for(int j=csr[i];j<csr[i+1];j++){
				temp+=aval[j]*m2[acol[j]];
				
		}		
			fm[i]=temp;
		
	}

		
	double stop; 
	stop= omp_get_wtime();
//	GET_TIME(stop);
//	printf("time %f \n",stop-start);
	     printf("Schedule: %s, Chunk Size: %d, Time: %f seconds\n", schedule_types[s], chunk_sizes[c], stop - start);
	}	
  }

}




