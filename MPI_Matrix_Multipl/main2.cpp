#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;
using namespace std::chrono;

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

int main() {
    srand(time(0));
    
    // Rozmiary macierzy takie same jak w wersji MPI
    int n = 3000; // liczba wierszy A
    int m = 3000; // liczba kolumn A = liczba wierszy B
    int p = 3000; // liczba kolumn B

    // Alokacja pamięci dla macierzy
    double** A = allocateMatrix(n, m);
    double** B = allocateMatrix(m, p);
    double** C = allocateMatrix(n, p);

    // Generowanie macierzy
    generateMatrix(A, n, m);
    generateMatrix(B, m, p);

    // Wyświetlanie macierzy wejściowych
    cout << "Macierz A (" << n << "x" << m << "):\n";
    //printMatrix(A, n, m);
    cout << "\nMacierz B (" << m << "x" << p << "):\n";
    //printMatrix(B, m, p);

    // Pomiar czasu rozpoczęcia
    auto start = high_resolution_clock::now();

    // Mnożenie macierzy
    multiplyMatrices(A, B, C, n, m, p);

    // Pomiar czasu zakończenia
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    // Wyświetlanie wyniku
    cout << "\nMacierz wynikowa C (" << n << "x" << p << "):\n";
    //printMatrix(C, n, p);

    // Wyświetlanie czasu wykonania
    cout << "\nCzas wykonania mnożenia: " 
         << duration.count() / 1000000.0 << " sekund\n";

    // Zwalnianie pamięci
    freeMatrix(A, n);
    freeMatrix(B, m);
    freeMatrix(C, n);

    return 0;
}