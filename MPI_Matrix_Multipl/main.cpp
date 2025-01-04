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
    int n = 8; 
    int m = 8; 
    int p = 8;

    if (m % size != 0) {
        if (rank == 0)
            cout << "Liczba procesów musi być dzielnikiem liczby kolumn pierwszej macierzy!" << endl;
        MPI_Finalize();
        return 1;
    }

    int rows_per_process = m / size;

    // a -> n, m
    // b -> m, p
    // c -> n, p


    // B is available to all processes
    double** B = allocateMatrix(m, p);
    generateMatrix(B, m, p);

    // Alokacja pamięci dla macierzy
    double** local_A = allocateMatrix(rows_per_process, m);
    double** local_C = allocateMatrix(rows_per_process, p);
    //macierz do rozsyłu danych
    double** A = nullptr;

    if (rank == 0) {
        //generowanie macierzy do rozsyłu danych wp rocesie 0
        A = allocateMatrix(n, m);
        generateMatrix(A, n, m);
        generateMatrix(B, m, p);

        cout << "Pierwsza macierz (" << n << "x" << m << "):\n";
        printMatrix(A, n, m);
    }

    if (rank == 0) {
      for(int i = 0; i < rows_per_process; i++) {
        for(int j = 0; j < m; j++) {
          local_A[i][j] = A[i][j];
        }
      }

      for(int i = 0; i < rows_per_process; i++) {
        MPI_Send(A[i + 2], m, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
      }

    for(int i = 0; i < rows_per_process; i++) {
        MPI_Send(A[i + 4], m, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD);
      }
    }

    if(rank == 1) {
      for(int i = 0; i < rows_per_process; i++) {
        MPI_Recv(local_A[i], m, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

        std::cout<<"Local Matrix Processor 1" << std::endl;
        printMatrix(local_A, rows_per_process, m);
    }

    if(rank == 2) {
      for(int i = 0; i < rows_per_process; i++) {
        MPI_Recv(local_A[i], m, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

        std::cout<<"Local Matrix Processor 2" << std::endl;
        printMatrix(local_A, rows_per_process, m);
    }
    
    
    if(rank == 0) {
        freeMatrix(A, n);
    }
    freeMatrix(local_A, n);
    freeMatrix(B, m);
    freeMatrix(local_C, n);
    
    MPI_Finalize();
    return 0;
}