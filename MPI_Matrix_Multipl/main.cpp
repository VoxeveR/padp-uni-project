#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

using namespace std;

// Funkcja do generowania macierzy o zadanych rozmiarach
vector<vector<int>> generateMatrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 10;  // Losowe liczby z zakresu 0-9
        }
    }
    return matrix;
}

// Funkcja do mnożenia macierzy
vector<vector<int>> multiplyMatrices(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int n = A.size();    // Liczba wierszy A
    int m = A[0].size(); // Liczba kolumn A i wierszy B
    int p = B[0].size(); // Liczba kolumn B

    // Wynikowa macierz wypełniona zerami
    vector<vector<int>> C(n, vector<int>(p, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < m; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return C;
}

// Funkcja do wyświetlania macierzy
void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << val << " ";
        }
        cout << endl;
    }
}

int main() {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(0)); // Inicjalizacja generatora liczb losowych
    int n = 10; // wiersze 1
    int m = 10; // kolumny 1 i wiersze 2
    int p = 10; // kolumny 2

    if (rank == 0) {
        vector<vector<int>> A = generateMatrix(n, m);
        vector<vector<int>> B = generateMatrix(m, p);

        cout << "Pierwsza macierz (" << n << "x" << m << "):\n";
        printMatrix(A);
        
        cout << "Druga macierz (" << m << "x" << p << "):\n";
        printMatrix(B);
    }
    vector<vector<int>> C = multiplyMatrices(A, B);

    if(rank == 0 ){
        cout << "Wynik mnożenia macierzy:\n";
        printMatrix(C);
    }
    
    MPI_Finalize();
    return 0;
}
