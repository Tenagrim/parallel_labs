#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

clock_t		get_elapsed_time(clock_t timer)
{
	return(clock() - timer);
}

clock_t		timer_start()
{
	return (clock());
}

int		arr_sum(int *arr, int size)
{
	int res = 0;

	for(int i = 0; i < size; i++)
		if(arr[i] > 0)
			res += arr[i];
	return (res);
}

void		arr_print(int *arr, int size)
{
	for(int i = 0; i < size; i++)
		printf("%d ", arr[i]);
	printf("\n");
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
		res[i] = (rand() % 200) - 100;
	if(sorted)
		qsort(res, size, sizeof(int), cmpfunc);	
	return res;
}


int		main(int ac, char **av)
{
	int	size;
	int	*arr_us;
	clock_t	timer;
	int	sum;

	if(ac < 2)	
	{
		printf("Invalid size\n");
		return (1);
	}
	size = atoi(av[1]);
	if(size <= 0)
	{
		printf("Invalid size\n");
		return (1);
	}
	srand(time(0));
	arr_us = arr_gen(size,0);

	timer = timer_start();
	sum = arr_sum(arr_us, size);
	printf("unsorted ticks: |%ld|  sum:  %d\n", get_elapsed_time(timer), sum);
	if(size < 100)
		arr_print(arr_us, size);

	qsort(arr_us, size, sizeof(int), cmpfunc);	
	timer = timer_start();
	sum = arr_sum(arr_us, size);
	printf("sorted   ticks: |%ld|  sum:  %d\n", get_elapsed_time(timer), sum);
	if(size < 100)
		arr_print(arr_us, size);

	
	printf("overall  ticks: |%ld|\n", clock() );
	free(arr_us);
}
