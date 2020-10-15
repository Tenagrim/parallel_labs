
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "mpi.h"
#include <omp.h>
using namespace std;

/*****************************FUNCTION PROTOTYPES******************************/

int inverse(int, int);
void find_inverses(int, int[]);
void swap(int*, int*, int);


class Space{
  public:
	int p;					//Prime defining Zp
	int* mult_inv;			//Array of multiplicative inverses in Zp
	int* add_inv;			//Array of additive inverses in Zp

	Space(int);
};

class Matrix {
  public:
	int **aug_matrix;		//Matrix being reduced
	int size;				//Number of rows

	void init_with_file(string, int);
	void display();
	void gauss(Space);
	void Resolve(Space);

};

void eliminate(int*, int*, int, int, Space);


/************************************MAIN**************************************/

int main (int argc, char **argv ) {
	int *pivot_row, *row;
	string filename;

	int numprocs, myid;

	MPI::Init(argc,argv);
	numprocs = MPI::COMM_WORLD.Get_size();
	myid = MPI::COMM_WORLD.Get_rank();

	int p = 7;
	Space modp(p);
		Matrix system;
		system.init_with_file("coefficients.txt", p);
		if(myid == 0){
			cout << endl << system.size << endl << endl;
			system.display();
		}
		system.Resolve(modp);
		if(myid == 0){
		system.display();
		}
	MPI::Finalize();
	return 0;
}

/****************************FUNCTION DEFINITIONS******************************/

int inverse(int p, int num) {
	if (num == 1)
		return 1;

	if (p%num == 0) {
		cerr << "\nError: \n\tProvided value is not a prime number.\n" << endl;
		exit(1);
	}

	else {
		int n = num - inverse(num, p%num);
		return (n*p + 1)/num;
	}
}

void find_inverses(int p, int inv_arr[]) {
	for (int i = 1; i < p; i++)
		inv_arr[i] = inverse(p, i);
	return;
}

void swap(int* row_a, int* row_b, int size){
	int temp;
	for (int i = 0; i < size; i++){
		temp = row_a[i];
		row_a[i] = row_b[i];
		row_b[i] = temp;
	}
}

void eliminate(int* pivotrow, int *row, int pivot, int size, Space modp) {
	int n = modp.add_inv[row[pivot]%modp.p];
	for (int i = 0; i < size + 1; i++) {
		row[i] = row[i] + (pivotrow[i] * n) ;
		row[i] = row[i] %modp.p;
	}
}

Space::Space(int n) {
		p = n;
		int inv;
		mult_inv = new int[p];
		add_inv = new int[p];

		find_inverses(n, mult_inv);
		add_inv[0] = 0;
		for (int i = 1; i < p/2 + 1; i++) {
			inv = p - i;
			add_inv[i] = inv;
			add_inv[inv] = i;
		}
}
void Matrix::display() {
	int j;
	for (int i = 0; i < size; i++) {
		for (j = 0; j < size -1; j++)
			cout << aug_matrix[i][j] << " x"<<j+1<< " +\t";
		cout << aug_matrix[i][j] << " x"<<j+1<< "\t";
		cout <<"=\t"<<aug_matrix[i][j+1]<< endl;
	}
	cout <<"======================================================="<< endl;
	return;
}

void Matrix::init_with_file(string filename, int prime) {

	ifstream infile;
	int temp_val, rows = 0, take;
	string temp_line;
	bool tokenerror;

	infile.open(filename.c_str(), fstream::in);
	if (infile.fail()) {
		cerr << "\nError: \n\tFile could not be opened\n" << endl;
		exit(1);
	}
	while (getline (infile, temp_line)) {
		rows++;
	}
	infile.close();
	infile.open(filename.c_str(), fstream::in);
	if (infile.fail()) {
		cerr << "\nError: \n\tFile could not be opened\n" << endl;
		exit(1);
	}
	aug_matrix = new int*[rows];
	for (int i = 0; i < rows; i++){
		aug_matrix[i] = new int[rows + 1];
		for(int j = 0; j < rows + 1; j++){
			infile >> temp_val;
			aug_matrix[i][j] = temp_val % prime;
		}
	}
	size = rows;
	infile.close();
	return;
}

void Matrix::Resolve(Space modp){

	int numprocs, myid, tag = 0;

    numprocs = MPI::COMM_WORLD.Get_size();
    myid = MPI::COMM_WORLD.Get_rank();

	int temp_array[size + 1], *temp_array_pivot = new int[size + 1];
	int i, j, k, inv, j_row;

	for (i = 0; i < size; i++) {
		if(myid ==0){
			if (aug_matrix[i][i] == 0) {
				for (j = i + 1; j < size ; j++) {
					if (aug_matrix[j][i] != 0) {
						swap(aug_matrix[i], aug_matrix[j], size);
						break;
					}
				}
				if (j >= size) {
					cout << endl <<"System does not have a unique solution" << endl;
					exit(1);
				}
			}
		}
	}
	// Прямой проход
	for(i = 0; i < size; i++){
		if(myid == 0){
			if (aug_matrix[i][i] != 1) {
				inv = modp.mult_inv[aug_matrix[i][i]];
				for (j = 0; j < size + 1; j++) {
					aug_matrix[i][j] = (aug_matrix[i][j] * inv) % modp.p;
				}
			}

			temp_array_pivot = aug_matrix[i];
			k = 1;
		}
			MPI::COMM_WORLD.Bcast(temp_array_pivot, size + 1, MPI::INT, 0);
			MPI::COMM_WORLD.Bcast(&i, 1, MPI::INT, 0);
		for (j = i + 1; j < size; j++){

			MPI::COMM_WORLD.Bcast(&k, 1, MPI::INT, 0);

			if(myid == 0){
					MPI::COMM_WORLD.Send (aug_matrix[j], size+1, MPI::INT, k, tag);
					MPI::COMM_WORLD.Send (&j, 1, MPI::INT, k, tag);
			}
			if(myid == k){ 
				MPI::COMM_WORLD.Recv(temp_array,size + 1, MPI::INT, 0, tag);
				MPI::COMM_WORLD.Recv(&j_row, 1, MPI::INT, 0, tag); 

				eliminate(temp_array_pivot, temp_array, i, size, modp); 

				MPI::COMM_WORLD.Send (&j_row, 1, MPI::INT, 0, tag);
				MPI::COMM_WORLD.Send (temp_array, size + 1, MPI::INT, 0, tag);
			}
							if(myid == 0){
								k++;
								if(k == numprocs){
									k = 1;
								}
							}
		}
		if(myid==0){
			k = 1;
		}
		for (j = i + 1; j < size; j++){
			if(myid == 0){
				MPI::COMM_WORLD.Recv(&j_row, 1, MPI::INT, k, tag);
				MPI::COMM_WORLD.Recv(aug_matrix[j_row],size + 1, MPI::INT, k, tag);
			}

			if(myid == 0){
				k++;
				if(k == numprocs){
					k = 1;
				}

			}
		}

		MPI::COMM_WORLD.Barrier();
	}
	MPI::COMM_WORLD.Barrier();
	//Обратный проход
	for (i = size-1; i >= 1; i--) {

		if(myid == 0){
			temp_array_pivot = aug_matrix[i];
			k=1;
		}
		MPI::COMM_WORLD.Bcast(temp_array_pivot, size + 1, MPI::INT, 0);
		MPI::COMM_WORLD.Bcast(&i, 1, MPI::INT, 0);
		for(j = i-1; j >= 0; j--) {
			MPI::COMM_WORLD.Bcast(&k, 1, MPI::INT, 0);
			if(myid == 0){
					MPI::COMM_WORLD.Send (aug_matrix[j], size+1, MPI::INT, k, tag);
					MPI::COMM_WORLD.Send (&j, 1, MPI::INT, k, tag); 
			}
			if(myid == k){ 
				MPI::COMM_WORLD.Recv(temp_array,size + 1, MPI::INT, 0, tag);
				MPI::COMM_WORLD.Recv(&j_row, 1, MPI::INT, 0, tag);
				eliminate(temp_array_pivot, temp_array, i, size, modp); 
				MPI::COMM_WORLD.Send (&j_row, 1, MPI::INT, 0, tag); 
				MPI::COMM_WORLD.Send (temp_array, size + 1, MPI::INT, 0, tag);
			}
							if(myid == 0){
								k++;
								if(k == numprocs){
									k = 1;
								}
							}
		}
		if(myid==0){
			k = 1;
		}
		for(j = i-1; j >= 0; j--) {
			if(myid == 0){
				MPI::COMM_WORLD.Recv(&j_row, 1, MPI::INT, k, tag); 
				MPI::COMM_WORLD.Recv(aug_matrix[j_row],size + 1, MPI::INT, k, tag); 
			}
			if(myid == 0){
				k++;
				if(k == numprocs){
					k = 1;
				}
			}
		}
		MPI::COMM_WORLD.Barrier();
	}
	MPI::COMM_WORLD.Barrier();
	return;
}

void Matrix::gauss(Space modp) {

	int numprocs, myid, tag = 0;

    numprocs = MPI::COMM_WORLD.Get_size();
    myid = MPI::COMM_WORLD.Get_rank();


	int i, j, k, inv;
	int temp_array[size + 1], *temp_array_pivot = new int[size + 1];

	for (i = 0; i < size; i++) {
		if(myid ==0){
		if (aug_matrix[i][i] == 0) {
			for (j = i + 1; j < size; j++) {
				if (aug_matrix[j][i] != 0) {
					swap(aug_matrix[i], aug_matrix[j], size);
					break;
				}
			}
			if (j >= size) {
				cout << endl <<"System does not have only one solution" << endl;
				exit(1);
			}
		}
		if (aug_matrix[i][i] != 1) {
			inv = modp.mult_inv[aug_matrix[i][i]];
			for (j = 0; j < size + 1; j++) {     //parallel for
				aug_matrix[i][j] = (aug_matrix[i][j] * inv) % modp.p;
			}
		}
		temp_array_pivot = aug_matrix[i];
	}
		MPI::COMM_WORLD.Bcast(temp_array_pivot, size + 1, MPI::INT, 0);
		MPI::COMM_WORLD.Bcast(&i, 1, MPI::INT, 0);
		if(myid == 0){
			k = 1;
		}
		for (j = i + 1; j < size; j++){
			MPI::COMM_WORLD.Bcast(&k, 1, MPI::INT, 0);
			if(myid == 0){
					MPI::COMM_WORLD.Send (aug_matrix[j], size+1, MPI::INT, k, tag);
			}
			if(myid == k){
				MPI::COMM_WORLD.Recv(temp_array,size + 1, MPI::INT, 0, tag);
				eliminate(temp_array_pivot, temp_array, i, size, modp); 
				MPI::COMM_WORLD.Send (temp_array, size + 1, MPI::INT, 0, tag);
			}
			if(myid == 0){
				k++;
				if(k == numprocs){
					k = 1;
				}
			}
		}
		if(myid == 0){
			k=1;
		}
		for (j = i + 1; j < size; j++){
			if(myid == 0){
				MPI::COMM_WORLD.Recv(aug_matrix[j],size + 1, MPI::INT, k, tag);
				for(int o =0; o < size+1; o++){
					cout << aug_matrix[j][o] << " ";
				}
				cout << endl;
			}
			if(myid == 0){
				k++;
				if(k == numprocs){
					k = 1;
				}
				display();
			}
		}
		MPI::COMM_WORLD.Barrier();
	}
	MPI::COMM_WORLD.Barrier();
	for (i = size-1; i >= 1; i--) {
		temp_array_pivot = aug_matrix[i];
		MPI::COMM_WORLD.Bcast(temp_array_pivot, size + 1, MPI::INT, 0);
		k=1;
		for(j = i-1; j >= 0; j--) {
				if(myid == 0){
					MPI::COMM_WORLD.Send (aug_matrix[j], size + 1, MPI::INT, k, tag);
				}
				if(myid == k){
					MPI::COMM_WORLD.Recv(temp_array,size + 1, MPI::INT, 0, tag);
					eliminate(temp_array_pivot, temp_array, i, size + 1, modp);
					MPI::COMM_WORLD.Send (temp_array, size + 1, MPI::INT, 0, tag);
				}
				k++;
				if(k == numprocs){
					k = 1;
				}
		}
		k = 1;
		for(j = i-1; j >= 0; j--) {
				if(myid == 0){
				MPI::COMM_WORLD.Recv(aug_matrix[j],size + 1, MPI::INT, k, tag);
				for(int o =0; o < size+1; o++){
						cout << aug_matrix[j][o] << " ";
					}
					cout << endl;
				}
				k++;
				if(k == numprocs){
					k = 1;
				}
			if(myid == 0){
				display();
			cout << endl << endl << endl;
			}
		}
		MPI::COMM_WORLD.Barrier();
	}
	return;
}

