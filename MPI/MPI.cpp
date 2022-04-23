#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "mpi.h"

#define FIRST_PROCESS 0

int main(int* argc, char** argv) {
	int process_size, rank;

	MPI_Init(argc, &argv);	//инициализация параллельной части программы

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//rank := номер процесса (от 0 до numtasks-1)
	MPI_Comm_size(MPI_COMM_WORLD, &process_size);	//numtasks := число параллельных процссов
	MPI_Status status;

	int* buff = NULL;
	int buff_size;

	if (rank == FIRST_PROCESS) { //если процесс первый
		printf(" >>> Num threads: %d\n", process_size);
		printf(" >>> Input mass size: ");
		fflush(stdout);	//очистка буфера
		int size;
		scanf("%d", &size);
	
		buff = new int[size];
		/*генерирование массива для пересылки*/
		for (int i = 0; i < size; i++) {
			*(buff + i) = i+1;
		}

		printf(" Hello MPI from process = %d, buff = [ ", rank);
		for (int i = 0; i < size; i++) {
			printf(" %d", *(buff + i));
		}
		printf(" ]\n");

		/*отправка всем процессам исходных данных с тегом сообщений 1 и 0*/
		for (int to_process = 1; to_process < process_size; to_process++) {
			MPI_Send(&size, 1, MPI_INT, to_process, 0, MPI_COMM_WORLD);	//размер массива
			MPI_Send(buff, size, MPI_INT, to_process, 1, MPI_COMM_WORLD);	//массив
		}
	}
	else {	//если процесс не первый, то ожидаем получения данных

		/*получение размера данных и выделение под них памяти*/
		MPI_Recv(&buff_size, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		buff = new int[buff_size];

		/*получение исходных данных*/
		MPI_Recv(buff, buff_size, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
		printf(" message form %d process, buff[7] = %d\n", status.MPI_SOURCE, *(buff+7));
	}


	MPI_Finalize();	//завершение параллельной части программы

	return 0;
}
