#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

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
   return ( *(int*)a - *(int*)b );
}
int		*arr_gen(int size, int sorted)
{
	int *res;
	res = malloc(sizeof(int) * size);

	for(int i = 0; i < size; i++)
		res[i] = (rand() % 20) - 10;
	if(sorted)
		qsort(res, size, sizeof(int), cmpfunc);	
	return res;
}
void		arr_print(int *arr, int size)
{
	for(int i = 0; i < size; i++)
		printf("%d ", arr[i]);
	printf("\n");
}
int		**matr_gen(matr_size size)
{
	int	**res;

	res = malloc(sizeof(int*) * size.rows);
	for(int i = 0; i < size.rows; i++)
		res[i] = arr_gen(size.cols, 0);
	return (res);
}
int		**matr_zero(matr_size size)
{
	int	**res;

	res = malloc(sizeof(int*) * size.rows);
	for(int i = 0; i < size.rows; i++)
	{
		res[i] = malloc(sizeof(int) * size.cols);
		for(int j = 0; j < size.cols; j++)
			res[i][j] = 0;
	}
	return (res);
}
int		matr_free(int **arr, matr_size size)
{
	for(int i = 0; i < size.rows; i++)
		free(arr[i]);
	free(arr);
}
int		matr_print(int **arr, matr_size size)
{
	for(int i = 0; i < size.rows; i++)
	{
		for(int j = 0; j < size.cols; j++)
			printf("%d ", arr[i][j]);
		printf("\n");
	}
}

int		*linearize_matrix(int **matrix, matr_size size, int transp)
{
	int *res = malloc(sizeof(int) * ( size.rows*size.cols));
	if(transp)
		for (int i = 0; i < size.rows; i++)
			for (int j = 0; j < size.cols; j++)
				res[i*size.cols + j] = matrix[j][i];
	else
		for (int i = 0; i < size.rows; i++)
			for (int j = 0; j < size.cols; j++)
				res[i*size.cols + j] = matrix[i][j];
	return res;
}

int**		matr_mult_ikj(int** a, int** b, matr_size size_a, matr_size size_b, matr_size *size_res)
{
	
	if (size_a.cols == size_b.rows)
	{
		size_res->rows = size_a.rows;
		size_res->cols = size_b.cols;
		int** result = matr_zero(*size_res);
		for (int i = 0; i < size_a.rows; i++)
			for (int k = 0; k < size_b.rows; k++)
				for (int j = 0; j < size_b.cols; j++) 
					result[i][j] += a[i][k] * b[k][j];
		return result;
	}
	return (NULL);
}
int	*zero_arr(int size)
{
	int *res = malloc(sizeof(int) * size);
	for(int i = 0; i < size; i++)
		res[i] = 0;
	return res;
}

int		*matr_mult_linear(int* matrix1, int* matrix2, int n1, int m1, int m2) {
	int *res = zero_arr(n1*m2);
	
	for (int i = 0; i < n1; i++)
		for (int j = 0; j < m2; j++)
			for (int k = 0; k < m1; k++) 
				res[i*m2 + j] += matrix1[i*m1 + k] * matrix2[j*m1 + k];
	return (res);
}
int		*matr_mult_linear_parallel(int *matrix1, int *matrix2, int n1, int m1, int m2) {
	int *res = zero_arr(n1*m2);
	
	#pragma omp parallel for schedule(static, 50)
	for (int i = 0; i < n1; i++)
		for (int j = 0; j < m2; j++)
			for (int k = 0; k < m1; k++) 
				res[i*m2 + j] += matrix1[i*m1 + k] * matrix2[j*m1 + k];
	return (res);
}
int	main(int ac, char **av)
{
	matr_size	size_a;
	matr_size	size_b;
	matr_size	size_res;
	int		**matr_a;
	int		**matr_b;
	int		**res;
	struct timeval	timer;
	int		*line_a;
	int		*line_b;
	int		*line_res;

	res = NULL;
	if(ac < 5)	
	{
		printf("Invalid sizes\n");
		return (1);
	}
	size_a.rows = atoi(av[1]);
	size_a.cols = atoi(av[2]);
	size_b.rows = atoi(av[3]);
	size_b.cols = atoi(av[4]);
	if(size_a.rows * size_b.rows * size_a.cols * size_b.cols == 0 || size_a.cols != size_b.rows)
	{
		printf("Invalid sizes\n");
		return (1);
	}
	srand(time(0));
	
	matr_a = matr_gen(size_a);
	matr_b = matr_gen(size_b);

	line_a = linearize_matrix(matr_a, size_a, 0);
	line_b = linearize_matrix(matr_b, size_b, 1);

	
	timer_start(&timer);
	res = matr_mult_ikj(matr_a, matr_b, size_a, size_b, &size_res);
	printf("consistent microseconds:  |%ld|   res[0][0]: %d\n", get_elapsed_time(timer), res[0][0]);

	timer_start(&timer);
	line_res = matr_mult_linear(line_a, line_b, size_a.rows, size_a.cols, size_b.cols);
	printf("linear	   microseconds:  |%ld|   res[0][0]: %d\n", get_elapsed_time(timer), line_res[0]);
	free(line_res);

	timer_start(&timer);
	line_res = matr_mult_linear_parallel(line_a, line_b, size_a.rows, size_a.cols, size_b.cols);
	printf("parallel   microseconds:  |%ld|   res[0][0]: %d\n", get_elapsed_time(timer), line_res[0]);

	matr_free(matr_a, size_a);
	matr_free(matr_b, size_b);
	matr_free(res, size_res);
	free(line_a);
	free(line_b);
	free(line_res);
	return (0);
}
