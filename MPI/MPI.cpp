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

/*выборка элементовы выше побочной диагонали*/
int* mpi_above_side_diagonal(int rank, int process_size, int** matrix, int n) {
	int size = (n * n - n) / 2;
	int* arr = (int*)malloc(size * sizeof(int));
	/*обнуление массива*/
	for (int i = 0; i < size; i++) {
		arr[i] = 0;
	}

	int interval = (int)(n / (process_size - 1));	//разбиение массива между процессами
	for (int i = (rank - 1) * interval; i < (rank - 1) * interval + interval && i < n; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			int counter = 0;
			int t = n;
			/*вычисление "своего места" в будующем массиве*/
			for (int k = 0; k < i; k++) {
				t = t - 1;
				counter += t;
			}
			arr[j + counter] = matrix[i][j];
		}
		/*особое уловие для предпоследней строки для некоторых матриц*/
		if (i == (process_size - 2) * interval) {
			arr[size - 1] = matrix[n - 2][0];
		}
	}

	return arr;
}

int main(int* argc, char** argv) {
	int process_size, rank;

	MPI_Init(argc, &argv);	//инициализация параллельной части программы

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//rank := номер процесса (от 0 до numtasks-1)
	MPI_Comm_size(MPI_COMM_WORLD, &process_size);	//numtasks := число параллельных процссов
	MPI_Status status;

	int** matrix = NULL;
	int matrix_parameters[2];	//высота и ширина матрицы
	int* result = NULL;
	int result_size = 0;
	int* arr = NULL;	//разобщённые резльтаты работы процессов

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
		result_size = (rows * rows - rows) / 2;
		result = (int*)malloc(result_size*sizeof(int));
		for (int i = 0; i < result_size; i++) {
			result[i] = 0;
		}

		/*заполнение матрицы значениями*/
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				//matrix[i][j] = rand() % 9 + 1;
				matrix[i][j] = (i + 1) * (j + 1);
			}
		}

		printf(" Hello MPI from process = %d, matrix: \n", rank);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				printf("\t%d", matrix[i][j]);
			}
			printf("\n");
		}

		/*отправка всем процессам исходных данных с тегом сообщений 1 и 0*/
		for (int to_process = 1; to_process < process_size; to_process++) {
			MPI_Send(matrix_parameters, 2, MPI_INT, to_process, 0, MPI_COMM_WORLD);	//размер массива
			MPI_Send(&(matrix[0][0]), rows * cols, MPI_INT, to_process, 1, MPI_COMM_WORLD);	//массив
		}

		arr = (int*)malloc(result_size * sizeof(int));
		for (int from_process = 2; from_process <= process_size; from_process++) {
			MPI_Recv(arr, result_size, MPI_INT, MPI_ANY_SOURCE, from_process, MPI_COMM_WORLD, &status);
			for (int i = 0; i < result_size; i++) {
				result[i] += arr[i];
			}
		}

		printf("\n >>> RESULT: [ ");
		for (int i = 0; i < result_size; i++) {
			printf(" %d", result[i]);
		}
		printf(" ]\n");

	}
	else {	//если процесс не первый, то ожидаем получения данных
		/*получение размера данных и выделение под них памяти*/
		MPI_Recv(matrix_parameters, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		matrix = matrix_init(matrix_parameters[0], matrix_parameters[1]);

		/*получение исходных данных*/
		MPI_Recv(&(matrix[0][0]), matrix_parameters[0] * matrix_parameters[1], MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
		result_size = ((matrix_parameters[0] * matrix_parameters[0] - matrix_parameters[0]) / 2);

		arr = mpi_above_side_diagonal(rank, process_size, matrix, matrix_parameters[0]);
		printf("\n rank: %d, arr: [ ", rank);
		for (int i = 0; i < result_size; i++) {
			printf(" %d", arr[i]);
		}
		printf(" ]\n");
		//отправка результата работы процесса в основной с тегами от 2 до 4 (в данном случе)
		MPI_Send(arr, result_size, MPI_INT, 0, rank + 1, MPI_COMM_WORLD);
	}

	MPI_Finalize();	//завершение параллельной части программы

	/*освобождение выделенной памяти*/
	free(matrix);
	free(result);
	free(arr);

	return 0;
}