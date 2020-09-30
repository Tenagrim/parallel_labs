PART_1 = part1
PART_2 = part2

all : $(PART_1) $(PART_2)

$(PART_1) : $(PART_1).c
	clear;
	gcc -fopenmp -O0 $(PART_1).c -o $(PART_1)

$(PART_2) : $(PART_2).c
	clear;
	gcc -fopenmp -O0 $(PART_2).c -o $(PART_2)


re: fclean all

clean : 
	rm -f *.o

fclean : clean
	rm -f $(PART_1) $(PART_2)
