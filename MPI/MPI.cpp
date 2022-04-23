#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <random>
#include <malloc.h>
#include "mpi.h"

#define FIRST_PROCESS 0

/*выделение памяти под двумерный массиав*/
int** matrix_init(int rows, int cols) {
	int* ptr = (int*)malloc(rows * cols * sizeof(int));
	int** matrix = (int**)malloc(rows * sizeof(int*));
	for (int i = 0; i < rows; i++) {
		matrix[i] = &(ptr[cols * i]);
	}
	return matrix;
}

int main(int* argc, char** argv) {
	int process_size, rank;

	MPI_Init(argc, &argv);	//инициализация параллельной части программы

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//rank := номер процесса (от 0 до numtasks-1)
	MPI_Comm_size(MPI_COMM_WORLD, &process_size);	//numtasks := число параллельных процссов
	MPI_Status status;

	int** matrix = NULL;
	int matrix_parameters[2];	//высота и ширина матрицы

	if (rank == FIRST_PROCESS) { //если процесс первый
		printf(" >>> Num threads: %d\n", process_size);

		printf(" >>> Input mass height: ");
		fflush(stdout);	//очистка буфера
		int rows;
		scanf("%d", &rows);
		matrix_parameters[0] = rows;

		printf(" >>> Input mass width: ");
		fflush(stdout);	//очистка буфера
		int cols;
		scanf("%d", &cols);
		matrix_parameters[1] = cols;

		matrix = matrix_init(rows, cols);

		/*заполнение матрицы значениями*/
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				matrix[i][j] = rand() % 9;
			}
		}

		printf(" Hello MPI from process = %d, matrix: \n", rank);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				printf(" %d", matrix[i][j]);
			}
			printf("\n");
		}

		/*отправка всем процессам исходных данных с тегом сообщений 1 и 0*/
		for (int to_process = 1; to_process < process_size; to_process++) {
			MPI_Send(matrix_parameters, 2, MPI_INT, to_process, 0, MPI_COMM_WORLD);	//размер массива
			MPI_Send(&(matrix[0][0]), rows * cols, MPI_INT, to_process, 1, MPI_COMM_WORLD);	//массив
		}
	}
	else {	//если процесс не первый, то ожидаем получения данных
		/*получение размера данных и выделение под них памяти*/
		MPI_Recv(matrix_parameters, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		matrix = matrix_init(matrix_parameters[0], matrix_parameters[1]);

		/*получение исходных данных*/
		MPI_Recv(&(matrix[0][0]), matrix_parameters[0] * matrix_parameters[1], MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
	}

	MPI_Finalize();	//завершение параллельной части программы

	return 0;
}