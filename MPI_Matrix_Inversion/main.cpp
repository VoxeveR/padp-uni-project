#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>

void generateRandomMatrix(std::vector<double>& matrix, int size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-10.0, 10.0);
    
    for(int i = 0; i < size * size; i++) {
        matrix[i] = dis(gen);
    }
}

void printMatrix(const std::vector<double>& matrix, int size) {
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(4) 
                      << matrix[i * size + j] << " ";
        }
        std::cout << std::endl;
    }
}

void swapRows(std::vector<double>& matrix, std::vector<double>& inverse, int size, int row1, int row2) {
    for(int i = 0; i < size; i++) {
        std::swap(matrix[row1 * size + i], matrix[row2 * size + i]);
        std::swap(inverse[row1 * size + i], inverse[row2 * size + i]);
    }
}

bool invertMatrixSequential(std::vector<double>& matrix, std::vector<double>& inverse, int size) {
    // Inicjalizacja macierzy jednostkowej jako początkowej macierzy odwrotnej
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            inverse[i * size + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Dla każdej kolumny
    for(int i = 0; i < size; i++) {
        // Znajdź największy element w kolumnie i (częściowe piwtowanie)
        double maxVal = std::abs(matrix[i * size + i]);
        int maxRow = i;
        for(int j = i + 1; j < size; j++) {
            if(std::abs(matrix[j * size + i]) > maxVal) {
                maxVal = std::abs(matrix[j * size + i]);
                maxRow = j;
            }
        }

        // Sprawdź czy macierz jest osobliwa
        if(maxVal < 1e-10) {
            return false;
        }

        // Zamień wiersze jeśli to konieczne
        if(maxRow != i) {
            swapRows(matrix, inverse, size, i, maxRow);
        }

        // Otrzymaj element na przekątnej
        double div = matrix[i * size + i];

        // Podziel wiersz przez element na przekątnej
        for(int j = 0; j < size; j++) {
            matrix[i * size + j] /= div;
            inverse[i * size + j] /= div;
        }

        // Odejmij od pozostałych wierszy
        for(int j = 0; j < size; j++) {
            if(j != i) {
                double mult = matrix[j * size + i];
                for(int k = 0; k < size; k++) {
                    matrix[j * size + k] -= matrix[i * size + k] * mult;
                    inverse[j * size + k] -= inverse[i * size + k] * mult;
                }
            }
        }
    }
    return true;
}

// Równoległa wersja obliczania macierzy odwrotnej
bool invertMatrixParallel(std::vector<double>& matrix, std::vector<double>& inverse, 
                         int size, int rank, int world_size) {
    // Inicjalizacja macierzy jednostkowej jako początkowej macierzy odwrotnej
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            inverse[i * size + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Oblicz ile wierszy przypada na każdy proces
    int rows_per_proc = size / world_size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == world_size - 1) ? size : start_row + rows_per_proc;

    // Główna pętla eliminacji Gaussa
    for(int i = 0; i < size; i++) {
        // Znajdź największy element w kolumnie i (częściowe piwtowanie)
        double local_max = 0.0;
        int local_max_row = -1;
        
        for(int j = std::max(i, start_row); j < end_row; j++) {
            if(std::abs(matrix[j * size + i]) > local_max) {
                local_max = std::abs(matrix[j * size + i]);
                local_max_row = j;
            }
        }


        // Znajdź globalnie największy element
        struct {
            double val;
            int row;
        } local_data, global_data;

        local_data.val = local_max;
        local_data.row = local_max_row;

        MPI_Allreduce(&local_data, &global_data, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);

        if(global_data.row != i && global_data.row != -1) {
            std::vector<double> temp_row(size);
            std::vector<double> temp_inv_row(size);

            if(rank == 0) {
                for(int k = 0; k < size; k++) {
                    temp_row[k] = matrix[i * size + k];
                    temp_inv_row[k] = inverse[i * size + k];
                }
            }

            if(rank == global_data.row / rows_per_proc) {
                MPI_Send(&matrix[global_data.row * size], size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
                MPI_Send(&inverse[global_data.row * size], size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
            }

            if(rank == 0) {
                MPI_Recv(&matrix[i * size], size, MPI_DOUBLE, 
                        global_data.row / rows_per_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&inverse[i * size], size, MPI_DOUBLE, 
                        global_data.row / rows_per_proc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                MPI_Send(temp_row.data(), size, MPI_DOUBLE, 
                        global_data.row / rows_per_proc, 2, MPI_COMM_WORLD);
                MPI_Send(temp_inv_row.data(), size, MPI_DOUBLE, 
                        global_data.row / rows_per_proc, 3, MPI_COMM_WORLD);
            }

            if(rank == global_data.row / rows_per_proc) {
                MPI_Recv(&matrix[global_data.row * size], size, MPI_DOUBLE, 
                        0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&inverse[global_data.row * size], size, MPI_DOUBLE, 
                        0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        if(std::abs(matrix[i * size + i]) < 1e-10) {
            return false;
        }

        MPI_Bcast(&matrix[i * size], size, MPI_DOUBLE, i / rows_per_proc, MPI_COMM_WORLD);
        MPI_Bcast(&inverse[i * size], size, MPI_DOUBLE, i / rows_per_proc, MPI_COMM_WORLD);

        double div = matrix[i * size + i];
        
        if(i >= start_row && i < end_row) {
            for(int j = 0; j < size; j++) {
                matrix[i * size + j] /= div;
                inverse[i * size + j] /= div;
            }
        }

        for(int j = start_row; j < end_row; j++) {
            if(j != i) {
                double mult = matrix[j * size + i];
                for(int k = 0; k < size; k++) {
                    matrix[j * size + k] -= matrix[i * size + k] * mult;
                    inverse[j * size + k] -= inverse[i * size + k] * mult;
                }
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        
        for(int proc = 0; proc < world_size; proc++) {
            int proc_start = proc * rows_per_proc;
            int proc_end = (proc == world_size - 1) ? size : proc_start + rows_per_proc;
            int proc_rows = proc_end - proc_start;
            
            MPI_Bcast(&matrix[proc_start * size], proc_rows * size, 
                     MPI_DOUBLE, proc, MPI_COMM_WORLD);
            MPI_Bcast(&inverse[proc_start * size], proc_rows * size, 
                     MPI_DOUBLE, proc, MPI_COMM_WORLD);
        }
    }
    return true;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Rozmiar macierzy (musi być podzielny przez liczbę procesów)
    const int matrix_size = 600;  // Zwiększyłem rozmiar dla lepszego porównania czasów

    if(matrix_size % world_size != 0) {
        if(world_rank == 0) {
            std::cout << "Rozmiar macierzy musi być podzielny przez liczbę procesów\n";
        }
        MPI_Finalize();
        return 1;
    }

    // Wektory dla wersji sekwencyjnej
    std::vector<double> matrix_seq;
    std::vector<double> inverse_seq;
    
    // Wektory dla wersji równoległej
    std::vector<double> matrix_par(matrix_size * matrix_size);
    std::vector<double> inverse_par(matrix_size * matrix_size);

    if(world_rank == 0) {
        // Inicjalizacja dla wersji sekwencyjnej
        matrix_seq.resize(matrix_size * matrix_size);
        inverse_seq.resize(matrix_size * matrix_size);
        generateRandomMatrix(matrix_seq, matrix_size);
        
        // Skopiuj tę samą macierz dla wersji równoległej
        matrix_par = matrix_seq;
        
        std::cout << "Rozmiar macierzy: " << matrix_size << "x" << matrix_size << "\n\n";
    }

    // Rozesłanie macierzy do wszystkich procesów dla wersji równoległej
    MPI_Bcast(matrix_par.data(), matrix_size * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Wykonaj wersję sekwencyjną tylko na procesie 0
    if(world_rank == 0) {
        double seq_start_time = MPI_Wtime();
        bool seq_success = invertMatrixSequential(matrix_seq, inverse_seq, matrix_size);
        double seq_end_time = MPI_Wtime();
        
        std::cout << "Czas wykonania wersji sekwencyjnej: " 
                  << seq_end_time - seq_start_time << " sekund\n";
    }

    // Synchronizacja przed rozpoczęciem wersji równoległej
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Wykonaj wersję równoległą
    double par_start_time = MPI_Wtime();
    bool par_success = invertMatrixParallel(matrix_par, inverse_par, matrix_size, world_rank, world_size);
    double par_end_time = MPI_Wtime();

    if(world_rank == 0) {
        std::cout << "Czas wykonania wersji równoległej: " 
                  << par_end_time - par_start_time << " sekund\n";
        std::cout << "Liczba użytych procesów: " << world_size << "\n";
    }

    MPI_Finalize();
    return 0;
}