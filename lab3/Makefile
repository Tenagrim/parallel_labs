NAME = lab3

all : $(NAME)

$(NAME): main.c
	clear;
	gcc -fopenmp -O3  main.c -o $(NAME)

re: fclean all

clean : 
	rm -f *.o

fclean : clean
	rm -f $(NAME)
