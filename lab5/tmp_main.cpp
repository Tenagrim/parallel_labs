//#include "/usr/include/mpich/mpi.h"
#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define VAR_COUNT 20

typedef struct
{
	int rows;
	int cols;
}	matr_size;
long		get_elapsed_time(struct timeval tvalBefore)
{
	struct timeval  tvalAfter;
	 gettimeofday (&tvalAfter, NULL);
	return((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L +tvalAfter.tv_usec) - tvalBefore.tv_usec;
}
void		timer_start(struct timeval *spec)
{
	gettimeofday(spec, NULL);
}
int 		cmpfunc(const void * a, const void * b) 
{
   return ( *(double*)a - *(double*)b );
}
double		*arr_gen(int size, int sorted)
{
	double *res;
	res = (double*)malloc(sizeof(double) * size);

	for(int i = 0; i < size; i++)
		res[i] = (double)((rand() % 20) - 10);
	if(sorted)
		qsort(res, size, sizeof(double), cmpfunc);	
	return res;
}

double		**matr_gen(matr_size size)
{
	double	**res;

	res = (double**)malloc(sizeof(double*) * size.rows);
	for(int i = 0; i < size.rows; i++)
		res[i] = arr_gen(size.cols, 0);
	return (res);
}
void		matr_free(double **arr, matr_size size)
{
	for(int i = 0; i < size.rows; i++)
		free(arr[i]);
	free(arr);
}

double *gauss(double **matrix, int n)
{
	double *res = (double*)malloc(sizeof(double)* n);
	double m;
	for (int k = 1; k < n; k++)
	{
		for (int j = k; j < n; j++)
		{
			m = (double)matrix[j][k - 1] / (double)matrix[k - 1][k - 1];
			for (int i = 0; i < n + 1; i++)
			{
				matrix[j][i] -= m * matrix[k - 1][i];
			}
		}
	}
	for (int i = n - 1; i >= 0; i--)
	{
		res[i] = matrix[i][n] / matrix[i][i];
		for (int c = n - 1; c > i; c--)
			res[i] -= matrix[i][c] * (res[c] / (double)matrix[i][i]);
	}
	return (res);
}
void	parallel_section(int start, int end, double **matrix, int n, int i)
{
	double tmp;
	int j, k;

	//printf("prepare\n");
	//printf("|%f|\n", matrix[0][0]);
	printf ("%d - %d | %d\n", start, end, i);
	for (j = start; j < end; j++)
	{
		      tmp = matrix[j][i];
		      for (k = n; k >= i; k--)
		      	matrix[j][k] -= tmp * matrix[i][k];
	}

}
void	gauss_parallel(double **matrix, int n, int procid, int numprocs, double **ress)
{
	double	tmp;
	int	i;
	int	j;
	int	k;
	double	*res;

	*ress = (double*)malloc(sizeof(double)* n);
	res = *ress;
		printf("<%d linear section>\n",procid);

	for (i = 0; i < n; i++)
	{
	// MASTER PART
	   if(1)//procid == 0)
	   {
	   	tmp = matrix[i][i];
	   	for (j = n; j >= i; j--)
	      		matrix[i][j] /= tmp;
	   }
	   // parallel part
	   int start = i + 1;
	   int end = n;
       	   

	   int proc_part = (end-start) / numprocs;
		printf("<%d parallel section p_part: %d  st: %d  en: %d>\n",procid, proc_part, start, end);
	   	parallel_section(procid * proc_part, (procid + 1) * proc_part, matrix, n, i);
	   	//parallel_section(start, end, matrix, n, i);
	}
	if(1)//procid == 0)
	{
	res[n - 1] = matrix[n - 1][n];
	for (i = n - 2; i >= 0; i--) {
	   res[i] = matrix[i][n];
	   for (j = i + 1; j < n; j++)
	      res[i] -= matrix[i][j] * res[j];
	}
	}
}

void	print_slau(double **m, matr_size size)
{
	int j;
	for(int i = 0; i < size.rows; i++)
	{
		for(j = 0; j < size.cols-2; j++)
			printf("(%d)x%d + ",(int)m[i][j], j + 1);
		printf("(%d)x%d = %d\n",(int)m[i][j], j + 1, (int)m[i][j+1]);
	}
}

int	main(int ac, char **av)
{
	int err = MPI_Init(&ac, &av);
	
	matr_size	size;
	double		**slau = NULL;
	double		*res;
	struct timeval	timer;
	int	procid;
	int	numprocs;
	int	tag = 1;
	MPI_Status	status;

	size.rows = VAR_COUNT;
	size.cols = VAR_COUNT + 1;
	MPI_Comm_rank(MPI_COMM_WORLD, &procid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	slau = matr_gen(size);
	printf("GEN END\n");
	printf("|%f|\n", slau[0][0]);
	if(procid == 0)
	{

	for(int dest = 1; dest < numprocs; dest++)
	{
		for(int i = 0; i < size.rows; i++)
		MPI_Send(slau[i],size.cols, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
		printf("Send to %d of %d <<%p>>\n", dest, numprocs, slau);
	}

	timer_start(&timer);
	//res = gauss(slau, size.rows);
	//print_slau(slau, size);
	//printf("consistent seconds: |%f|   res[0]: %f\n", (double)get_elapsed_time(timer)/1000000, res[0]);
	//free(res);
	

	}
	if(procid != 0)
	{
	printf("Start receiving\n");
	for(int i  = 0; i < size.rows;i++)
	MPI_Recv(slau[i], size.cols, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, &status);
	printf("Recieve im %d of %d <<%p>>\n", procid, numprocs, slau);
	printf("|%f|\n", slau[0][0]);
	}
	timer_start(&timer);
	//res = gauss(slau, size.rows);
	gauss_parallel(slau, size.rows, procid, numprocs, &res);
	if(procid == 0)
	{
	//resv
	}
	else
	{
	//send
	}
	MPI_Finalize();
	if(procid == 500)
	{
	printf("<%d>\n", procid);
	printf("parallel   seconds: |%f|   res[0]: %f\n", (double)get_elapsed_time(timer)/1000000, res[0]);
	res = gauss(slau, size.rows);
	printf("parallel   seconds: |%f|   res[0]: %f\n", (double)get_elapsed_time(timer)/1000000, res[0]);
	if(size.rows <= 100)
	{
		for(int i = 0; i < size.rows; i++)
			printf("x%d = %f\n",i+1,res[i]);
		printf("\n");
	}
	//free(res);
	}
	//matr_free(slau,size);
	return (0);
}
