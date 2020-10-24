// Подключение необходимых заголовков
#include <stdio.h>
#include <math.h>
// Подключение заголовочного файла MPI
#include "mpi.h"

const int size = 100000000;

// Главная функция программы
int main(int argc, char** argv)
{
	int myid, np;
	int namelen;
	char proc_nam[MPI_MAX_PROCESSOR_NAME];


	// Инициализация подсистемы MPI
	MPI_Init(&argc, &argv);

	// Получить размер коммуникатора MPI_COMM_WORLD
	// (общее число процессов в рамках задачи)
	MPI_Comm_size(MPI_COMM_WORLD, &np);

	// Получить номер текущего процесса в рамках 
	// коммуникатора MPI_COMM_WORLD
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Get_processor_name(proc_nam, &namelen);


	// Объявление переменных
	int done = 0, n, i;
	double mypi, sumlocal, sumall;
	double t1s, t2s, t1p, t2p;
	double* a, * sum;
	MPI_Status st;

	sum = new double[np];
	sumlocal = 0;
	sumall = 0;

	if (myid == 0)
	{
		printf("Nbr of elements is %d\n", size);
		a = new double[size];
		for (int i = 0; i < size; i++)
			a[i] = sin(i);
	}
	else
		a = new double[size / np];

	if (myid == 0)
	{
		double sumcon = 0;

		t1s = MPI_Wtime();
		for (int i = 0; i < size; i++)
			sumcon += cos(sqrt(exp(0.5 * log(sin(a[i]) + 2.0))));
		t2s = MPI_Wtime();
		printf("without parallel sum = %f, time = %f\n", sumcon, t2s - t1s);
	}

	t1p = MPI_Wtime();
	//передача частей массива из нулевого в остальные
	if (myid == 0)
		for (int i = 1; i < np; i++)
			MPI_Send(&a[i * (size / np)], size / np, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
	else
		MPI_Recv(a, size / np, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &st);

	for (int i = 0; i < size / np; i++)
		sumlocal += cos(sqrt(exp(0.5 * log(sin(a[i]) + 2.0))));

	if (myid != 0)
		MPI_Send(&sumlocal, 1, MPI_DOUBLE, 0, myid, MPI_COMM_WORLD);
	else
	{
		for (int i = 1; i < np; i++)
		{
			MPI_Recv(&sumall, 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &st);
			sumlocal += sumall;
		}
	}
	t2p = MPI_Wtime();
	if (myid == 0)
	{
		printf("with parallel sum = %f, time = %f\n", sumlocal, t2p - t1p);
		printf("without parallel vs parallel times = %f\n", (t2s - t1s) / (t2p - t1p));
	}


	// Освобождение подсистемы MPI
	MPI_Finalize();
	return 0;
}