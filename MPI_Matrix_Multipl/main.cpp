#include <iostream>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

using namespace std;

// Funkcja do generowania macierzy o zadanych rozmiarach
void generateMatrix(double** matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 10;  // Losowe liczby z zakresu 0-9
        }
    }
}

// Funkcja do mnożenia macierzy
void multiplyMatrices(double** A, double** B, double** C, int n, int m, int p) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < m; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Funkcja do wyświetlania macierzy
void printMatrix(double** matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

// Funkcja do alokacji pamięci dla macierzy
double** allocateMatrix(int rows, int cols) {
    double** matrix = new double*[rows];

    for (int i = 0; i < rows; ++i) {
        matrix[i] = new double[cols];
    }
    return matrix;
}

// Funkcja do zwalniania pamięci
void freeMatrix(double** matrix, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(0)); 
    int n = 3000; // rowsA
    int m = 3000; // colsA, rowsB
    int p = 3000; // colsB

    if (n % size != 0) {
        if (rank == 0)
            cout << "Liczba procesów musi być dzielnikiem liczby kolumn pierwszej macierzy!" << endl;
        MPI_Finalize();
        return 1;
    }

    int rows_per_process = n / size;

    // a -> n, m
    // b -> m, p
    // c -> n, p


    // B is available to all processes
    double** B = allocateMatrix(m, p);
    generateMatrix(B, m, p);

    // Alokacja pamięci dla macierzy
    double** local_A;
    double** local_B;
    double** local_C = allocateMatrix(rows_per_process, p);
    //macierz do rozsyłu danych
    double** A = nullptr;
    double** C = nullptr;
    
    double start_time, end_time;

    if(rank == 0) {
        start_time = MPI_Wtime();
    }

    if (rank == 0) {
        //generowanie macierzy do rozsyłu danych wp rocesie 0
        A = allocateMatrix(n, m);
        C = allocateMatrix(n, p);
        generateMatrix(A, n, m);

       // cout << "Pierwsza macierz (" << n << "x" << m << "):\n";
       // printMatrix(A, n, m);
        
       // cout << "Druga macierz (" << n << "x" << m << "):\n";
        //printMatrix(B, n, m);
    }


    if (rank == 0) {
      local_A = allocateMatrix(rows_per_process, m);
      local_C = allocateMatrix(rows_per_process, p);
      for(int i = 0; i < rows_per_process; i++) {
        for(int j = 0; j < m; j++) {
          local_A[i][j] = A[i][j];
        }
      }

        //sending to processor 1
        for(int i = 0; i < rows_per_process; i++) {
            MPI_Send(A[i + rows_per_process], m, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        }

        //sending to processor 2
        for(int i = 0; i < rows_per_process * 2; i++) {
            MPI_Send(A[i + rows_per_process * 2], m, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD);
        } 
    }

    if(rank == 1) {
        local_A = allocateMatrix(rows_per_process, m);
        local_C = allocateMatrix(rows_per_process, p);

        for(int i = 0; i < rows_per_process; i++) {
            MPI_Recv(local_A[i], m, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

    }


    if(rank == 2) {
        local_A = allocateMatrix(rows_per_process*2, m);
        local_C = allocateMatrix(rows_per_process*2, p);

      for(int i = 0; i < rows_per_process*2; i++) {
        MPI_Recv(local_A[i], m, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

      for(int i = 0; i < rows_per_process; i++){
        MPI_Send(local_A[i + rows_per_process], m, MPI_DOUBLE, 3, 0, MPI_COMM_WORLD);
      }

    }

    if(rank == 3){
      local_A = allocateMatrix(rows_per_process, m);
      local_C = allocateMatrix(rows_per_process, p);

      for(int i = 0; i < rows_per_process; i++) {
        MPI_Recv(local_A[i], m, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

    }

    /*
    for(int i = 0; i < size; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == i) {
            cout << "\nLokalna część macierzy A dla procesu " << rank << ":\n";
            int rows_to_print = (rank == 2) ? rows_per_process * 2 : rows_per_process;
            printMatrix(local_A, rows_to_print, m);
        }
    }
    */

    multiplyMatrices(local_A, B, local_C, rows_per_process, m, p);

    /*
    // Wyświetlanie lokalnych wyników
    for(int i = 0; i < size; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == i) {
            cout << "\nWynik lokalnego mnożenia dla procesu " << rank << ":\n";
            int rows_to_print = (rank == 2) ? rows_per_process * 2 : rows_per_process;
            printMatrix(local_C, rows_to_print, p);
        }
    }
    */

    if(rank == 3) {
        // Proces 3 wysyła do 2
        for(int i = 0; i < rows_per_process; i++) {
            MPI_Send(local_C[i], p, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD);
        }
    }

    if(rank == 2) {
        // Proces 2 odbiera od 3
        double** temp_C = allocateMatrix(rows_per_process, p);
        for(int i = 0; i < rows_per_process; i++) {
            MPI_Recv(temp_C[i], p, MPI_DOUBLE, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // Kopiowanie danych od procesu 3 do właściwego miejsca
        for(int i = 0; i < rows_per_process; i++) {
            for(int j = 0; j < p; j++) {
                local_C[i + rows_per_process][j] = temp_C[i][j];
            }
        }
        
        // Wysyłanie całości do procesu 0
        for(int i = 0; i < rows_per_process * 2; i++) {
            MPI_Send(local_C[i], p, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
        freeMatrix(temp_C, rows_per_process);
    }

    if(rank == 1) {
        // Proces 1 wysyła do 0
        for(int i = 0; i < rows_per_process; i++) {
            MPI_Send(local_C[i], p, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    if(rank == 0) {
        // Kopiowanie własnych wyników
        for(int i = 0; i < rows_per_process; i++) {
            for(int j = 0; j < p; j++) {
                C[i][j] = local_C[i][j];
            }
        }

        // Odbieranie od procesu 1
        for(int i = 0; i < rows_per_process; i++) {
            MPI_Recv(&C[rows_per_process + i][0], p, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Odbieranie od procesu 2 (zawiera też wyniki procesu 3)
        for(int i = 0; i < rows_per_process * 2; i++) {
            MPI_Recv(&C[rows_per_process * 2 + i][0], p, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        //cout << "\nKońcowa macierz C (" << n << "x" << p << "):\n";
        //printMatrix(C, n, p);
    }

    if(rank == 0) {
        end_time = MPI_Wtime();
        cout << "\nCzas wykonania mnożenia macierzy: " << (end_time - start_time) << " sekund\n";
    }

    // Zwalnianie pamięci
    if(rank == 0) {
        freeMatrix(A, n);
        freeMatrix(C, n);
    }
    freeMatrix(B, m);
    if(rank == 2) {
        freeMatrix(local_A, rows_per_process * 2);
        freeMatrix(local_C, rows_per_process * 2);
    } else {
        freeMatrix(local_A, rows_per_process);
        freeMatrix(local_C, rows_per_process);
    }
    
    MPI_Finalize();
    return 0;
}