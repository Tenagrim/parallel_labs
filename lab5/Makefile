NAME = lab5

all : $(NAME)

$(NAME): main.cpp
	mpic++ -O0 main.cpp -o $(NAME)

re: fclean all

clean :
	rm -f *.o

fclean : clean
	rm -f $(NAME)
