#include <iostream>
#include "mpi.h"


void Print_Matrix(int* Mat, int size)
{
    std::cout << "Multiplication: " << std::endl;
    for (int i=0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            std::cout << Mat[i*size + j] << " ";
        }
        std::cout << std::endl;
    }
}


int main(int argc,char *argv[])
{
    double start, stop;
    int i, j, k, l;
    int *A, *B, *C, *buffer, *ans;
    int size = 1000;
    int rank, numprocs, line;

    MPI_Init(&argc,&argv); //MPI Initialize
    MPI_Comm_rank(MPI_COMM_WORLD,&rank); // Получить текущий номер процесса
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs); // Получить количество процессов

    line = size/numprocs; // Делим данные на блоки (количество процессов), и основной процесс также должен обрабатывать данные
    A = new int[size*size];
    B = new int[size*size];
    C = new int[size*size];
    // Размер кеша больше или равен размеру обрабатываемых данных, когда он больше, чем фактическая часть данных
    buffer = new int[size*line]; // Размер пакета данных
    ans = new int[size*line]; // Сохраняем результат расчета блока данных

    // Основной процесс присваивает матрице начальное значение и передает матрицу N каждому процессу, а матрицу M передает каждому процессу в группах.
    if (rank == 0)
    {
        for(i = 0; i < size; i++) // Чтение данных
            for(j = 0; j < size; j++)
                A[i * size + j] = (i + 2) * (j + 1);

        for(i = 0; i < size; i++)
            for(j = 0; j < size; j++)
                B[i * size + j] = (i + 1) + 2 * (j + 3);

        start = MPI_Wtime();
        // Отправить матрицу N другим подчиненным процессам
        for (i = 1; i < numprocs; i++) {
            MPI_Send(B,size*size,MPI_INT,i,0,MPI_COMM_WORLD);
        }
        // Отправляем каждую строку a каждому подчиненному процессу по очереди
        for (l = 1; l < numprocs; l++) {
            MPI_Send(A+(l-1)*line*size,size*line,MPI_INT,l,1,MPI_COMM_WORLD);
        }
        // Получаем результат, рассчитанный по процессу
        for (k = 1; k < numprocs; k++) {
            MPI_Recv(ans,line*size,MPI_INT,k,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            // Передаем результат в массив c
            for (i = 0; i < line; i++) {
                for (j = 0; j < size; j++) {
                    C[((k-1)*line+i)*size+j] = ans[i*size+j];
                }
            }
        }
        // Рассчитать оставшиеся данные
        for (i = (numprocs-1) * line;i < size; i++) {
            for (j = 0; j < size; j++) {
                int temp=0;
                for (k = 0; k < size; k++)
                    temp += A[i*size+k]*B[k*size+j];
                C[i*size+j] = temp;
            }
        }

        // Результат теста
        // Статистика по времени
        stop = MPI_Wtime();

        std::cout << "Rank: " << rank << "\nTime: " << stop-start << " s" << std::endl;

        //Print_Matrix(C, size);

        delete [] A;
        delete [] B;
        delete [] C;
        delete [] buffer;
        delete [] ans;
    }

        // Другие процессы получают данные и после вычисления результата отправляют их в основной процесс
    else {
        // Получаем широковещательные данные (матрица b)
        MPI_Recv(B,size*size,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        MPI_Recv(buffer,size*line,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        // Рассчитать результат продукта и отправить результат в основной процесс
        for (i = 0; i < line; i++) {
            for (j = 0; j < size; j++) {
                int temp=0;
                for(k = 0; k < size; k++)
                    temp += buffer[i*size+k]*B[k*size+j];
                ans[i*size+j]=temp;
            }
        }
        // Отправить результат расчета в основной процесс
        MPI_Send(ans,line*size,MPI_INT,0,3,MPI_COMM_WORLD);
    }

    MPI_Finalize(); // Конец

    return 0;
}