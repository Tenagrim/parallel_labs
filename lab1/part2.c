#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct
{
	int rows;
	int cols;
}	matr_size;

clock_t		get_elapsed_time(clock_t timer)
{
	return(clock() - timer);
}

clock_t		timer_start()
{
	return (clock());
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

int** matr_mult_ijk(int** a, int** b, matr_size size_a, matr_size size_b, matr_size *size_res)
{
	
	if (size_a.cols == size_b.rows)
	{
		size_res->rows = size_a.rows;
		size_res->cols = size_b.cols;
		int** result = matr_zero(*size_res);
		for (int i = 0; i < size_a.rows; i++)
			for (int j = 0; j < size_b.cols; j++) 
				for (int k = 0; k < size_b.rows; k++)
					result[i][j] += a[i][k] * b[k][j];
		return result;
	}
	return (NULL);
}
int** matr_mult_ikj(int** a, int** b, matr_size size_a, matr_size size_b, matr_size *size_res)
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
int** matr_mult_kij(int** a, int** b, matr_size size_a, matr_size size_b, matr_size *size_res)
{
	
	if (size_a.cols == size_b.rows)
	{
		size_res->rows = size_a.rows;
		size_res->cols = size_b.cols;
		int** result = matr_zero(*size_res);
		for (int k = 0; k < size_b.rows; k++)
			for (int i = 0; i < size_a.rows; i++)
				for (int j = 0; j < size_b.cols; j++) 
					result[i][j] += a[i][k] * b[k][j];
		return result;
	}
	return (NULL);
}
int		main(int ac, char **av)
{
	matr_size	size_a;
	matr_size	size_b;
	matr_size	size_res;
	int		**matr_a;
	int		**matr_b;
	int		**res;
	clock_t		timer;

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
	
	timer = timer_start();
	res = matr_mult_ijk(matr_a, matr_b, size_a, size_b, &size_res);
	printf("ijk order ticks:  |%ld|   res[0][0]: %d\n", get_elapsed_time(timer), res[0][0]);
	matr_free(res, size_res);

	timer = timer_start();
	res = matr_mult_ikj(matr_a, matr_b, size_a, size_b, &size_res);
	printf("ikj order ticks:  |%ld|   res[0][0]: %d\n", get_elapsed_time(timer), res[0][0]);

	timer = timer_start();
	res = matr_mult_kij(matr_a, matr_b, size_a, size_b, &size_res);
	printf("kij order ticks:  |%ld|   res[0][0]: %d\n", get_elapsed_time(timer), res[0][0]);

	if(size_res.rows <= 5 && size_res.cols <= 5)
	{
		printf("A ===========\n");
		matr_print(matr_a, size_a);
		printf("B ===========\n");
		matr_print(matr_b, size_b);
		printf("=============\n");

		printf("res: %d %d\n", size_res.rows,size_res.cols);
		printf("RES =========\n");
		matr_print(res, size_res);
		printf("=============\n");
	}
	matr_free(matr_a, size_a);
	matr_free(matr_b, size_b);
	if(res)
		matr_free(res, size_res);
	return(0);
}
