NAME = lab4
TBB_INCLUDE=oneTBB/include
TBB_LIB=oneTBB/build/linux_intel64_gcc_cc7.5.0_libc2.27_kernel5.4.0_release
all : $(NAME)

$(NAME): main.cpp
	clear;
	g++ -g main.cpp -o $(NAME) -I$(TBB_INCLUDE) -Wl,-rpath,$(TBB_LIB) -L$(TBB_LIB) -ltbb  

re: fclean all

clean :
	rm -f *.o

fclean : clean
	rm -f $(NAME)
