#include <mpi.h>
#include <iostream>
#include <vector>
#include <iomanip>

#define N 8  // Rozmiar macierzy

void printMatrix(double* matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << std::fixed << std::setprecision(4) << std::setw(8) 
                      << matrix[i * n + j] << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Start pomiaru czasu
    start_time = MPI_Wtime();

    // Sprawdzenie czy liczba procesów jest odpowiednia
    if (N % size != 0) {
        if (rank == 0)
            std::cout << "Liczba procesów musi być dzielnikiem rozmiaru macierzy!" << std::endl;
        MPI_Finalize();
        return 1;
    }

    int rows_per_proc = N / size;
    
    // Alokacja pamięci dla macierzy lokalnych
    double* A_local = new double[rows_per_proc * N];
    double* I_local = new double[rows_per_proc * N];
    
    // Proces główny inicjalizuje macierze
    double *A = nullptr, *I = nullptr;
    if (rank == 0) {
        A = new double[N * N];
        I = new double[N * N];
        
        // Inicjalizacja macierzy A i I
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A[i * N + j] = j*i + rand() % 10;
                I[i * N + j] = (i == j) ? 1.0 : 0.0;
            }
        }
        
        std::cout << "Macierz początkowa:\n";
        printMatrix(A, N);
    }

    // Rozdzielenie danych
    MPI_Scatter(A, rows_per_proc * N, MPI_DOUBLE, A_local, rows_per_proc * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(I, rows_per_proc * N, MPI_DOUBLE, I_local, rows_per_proc * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Bufor dla wiersza pivot
    double* pivot_row_A = new double[N];
    double* pivot_row_I = new double[N];

    // Eliminacja Gaussa-Jordana
    for (int k = 0; k < N; k++) {
        int pivot_proc = k / rows_per_proc;
        int local_row = k % rows_per_proc;

        // Proces zawierający wiersz pivot wysyła go do wszystkich
        if (rank == pivot_proc) {
            for (int j = 0; j < N; j++) {
                pivot_row_A[j] = A_local[local_row * N + j];
                pivot_row_I[j] = I_local[local_row * N + j];
            }
        }

        // Broadcast wiersza pivot
        MPI_Bcast(pivot_row_A, N, MPI_DOUBLE, pivot_proc, MPI_COMM_WORLD);
        MPI_Bcast(pivot_row_I, N, MPI_DOUBLE, pivot_proc, MPI_COMM_WORLD);

        // Obliczenia na lokalnych wierszach
        for (int i = 0; i < rows_per_proc; i++) {
            if (rank * rows_per_proc + i != k) {
                double factor = A_local[i * N + k] / pivot_row_A[k];
                for (int j = 0; j < N; j++) {
                    A_local[i * N + j] -= factor * pivot_row_A[j];
                    I_local[i * N + j] -= factor * pivot_row_I[j];
                }
            }
        }
    }

    // Normalizacja
    for (int i = 0; i < rows_per_proc; i++) {
        int global_row = rank * rows_per_proc + i;
        double pivot = A_local[i * N + global_row];
        for (int j = 0; j < N; j++) {
            I_local[i * N + j] /= pivot;
        }
    }

    // Zebranie wyników
    double* result = nullptr;
    if (rank == 0) {
        result = new double[N * N];
    }

    MPI_Gather(I_local, rows_per_proc * N, MPI_DOUBLE, result, rows_per_proc * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Koniec pomiaru czasu
    end_time = MPI_Wtime();

    // Wyświetlenie wyniku i czasu wykonania
    if (rank == 0) {
        std::cout << "\nMacierz odwrócona:\n";
        printMatrix(result, N);
        
        std::cout << "\nCzasy wykonania:\n";
        std::cout << "Całkowity czas wykonania: " << end_time - start_time << " sekund\n";
        std::cout << "Średni czas na proces: " << (end_time - start_time) / size << " sekund\n";
        
        delete[] result;
        delete[] A;
        delete[] I;
    }

    // Zwolnienie pamięci
    delete[] A_local;
    delete[] I_local;
    delete[] pivot_row_A;
    delete[] pivot_row_I;

    MPI_Finalize();
    return 0;
}