#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

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
	res = (double *)malloc(sizeof(double) * size);

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
double		**matr_copy(double **src , matr_size size)
{
	double	**res;

	res = (double**)malloc(sizeof(double*) * size.rows);
	for(int i = 0; i < size.rows; i++)
	{
		res[i] = (double*)malloc(sizeof(double) * size.cols); 
		for(int j = 0; j < size.cols; j++)
			res[i][j] = src[i][j];
	}
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

class solver
{
	public:
		double	**matrix;
		int	rows;
		int	cols;
		int	i;

		solver(double** matrix, int rows, int cols)
		{
			this->matrix = matrix;
			this->rows = rows;
			this->cols = cols;
		}
		void operator() (const tbb::blocked_range<int>&range) const
		{
			//forward
			double	tmp;
			int n = rows;
			int k;
			for (int j = range.begin(); j < range.end(); j++)
			{
		      		tmp = matrix[j][i];
		      		for (k = n; k >= i; k--)
		      			matrix[j][k] -= tmp * matrix[i][k];
			}

		}
		double *backward()
		{
			double *res = (double*)malloc(sizeof(double)* rows);
			int i;
			int j;
			int n = rows;
			res[n - 1] = matrix[n - 1][n];
			for (i = n - 2; i >= 0; i--) {
			res[i] = matrix[i][n];
			   for (j = i + 1; j < n; j++)
			      res[i] -= matrix[i][j] * res[j];
			}
			return (res);
		}

};
double *parallel_solve(solver *s)
{
	double tmp;
	int i,j;
	int n = s->rows;

	for (i = 0; i < n; i++)
	{
	   tmp = s->matrix[i][i];
	   for (j = n; j >= i; j--)
	      s->matrix[i][j] /= tmp;
	   tbb::parallel_for(tbb::blocked_range<int>(i+1, n), *s);
	}
	return (s->backward());
}

int	main(int ac, char **av)
{
	matr_size	size;
	double		**slau1;
	double		**slau2;
	double		*res;
	struct timeval	timer;

	if(ac < 2)
	{
		printf("Invalid sizes\n");
		return (1);
	}
	size.rows = atoi(av[1]);
	size.cols = size.rows+1;
	if(size.rows <= 0)
	{
		printf("Invalid sizes\n");
		return (1);
	}
	//srand(time(0));
	slau1 = matr_gen(size);
	slau2 = matr_copy(slau1, size);
	if(size.rows <= 10)
	{
		print_slau(slau1,size);
		printf("\n");
		print_slau(slau1,size);
		printf("\n");
	}
	solver		*s = new solver(slau1, size.rows, size.cols);
	timer_start(&timer);
	res = gauss(slau1, size.rows);
	printf("consistent seconds: |%f|   res[0]: %f\n", (double)get_elapsed_time(timer)/1000000, res[0]);
	free(res);

	timer_start(&timer);
	res = parallel_solve(s);
	printf("parallel   seconds: |%f|   res[0]: %f\n", (double)get_elapsed_time(timer)/1000000, res[0]);
	if(size.rows <= 10)
	{
		for(int i = 0; i < size.rows; i++)
			printf("x%d = %f\n",i+1,res[i]);
		printf("\n");
	}
	free(res);

	matr_free(slau1,size);
	matr_free(slau2,size);
	return (0);
}
