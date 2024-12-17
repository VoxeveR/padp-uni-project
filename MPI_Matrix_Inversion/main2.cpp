#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>
#include <chrono>

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


int main(int argc, char** argv) {
    int N = 10;
    std::vector<double> matrix_seq(N*N);
    std::vector<double> inverse_seq(N*N);

    generateRandomMatrix(matrix_seq, N);
    std::cout << "Macierz podstawa\n";
    printMatrix(matrix_seq, N);
    
    auto start_seq = std::chrono::high_resolution_clock::now();
    bool success = invertMatrixSequential(matrix_seq, inverse_seq, N);
    auto end_seq = std::chrono::high_resolution_clock::now();

    

    std::chrono::duration<double> time_seq = end_seq - start_seq;
    std::cout << "Macierz odwrócona\n";
    printMatrix(inverse_seq, N);
    
    std::cout << "\nCzas wykonania (sekwencyjny): " << time_seq.count() << " sekund\n";

    return 0;
}