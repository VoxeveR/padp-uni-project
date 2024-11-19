import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.*;
import java.util.stream.IntStream;

public class GaussEliminationComparison {

    public static void gaussEliminationSequential(double[][] matrix, double[] rhs) {
        int n = matrix.length;

        for (int k = 0; k < n; k++) {
            // Normalizacja wiersza k
            double pivot = matrix[k][k];
            for (int j = k; j < n; j++) {
                matrix[k][j] /= pivot;
            }
            rhs[k] /= pivot;

            // Eliminacja wierszy poniżej k
            for (int i = k + 1; i < n; i++) {
                double factor = matrix[i][k];
                for (int j = k; j < n; j++) {
                    matrix[i][j] -= factor * matrix[k][j];
                }
                rhs[i] -= factor * rhs[k];
            }
        }

        // Rozwiązanie układu równań - metoda wsteczna
        double[] solution = new double[n];
        for (int i = n - 1; i >= 0; i--) {
            solution[i] = rhs[i];
            for (int j = i + 1; j < n; j++) {
                solution[i] -= matrix[i][j] * solution[j];
            }
        }
    }

    public static void gaussEliminationParallel(double[][] matrix, double[] rhs) {
        int n = matrix.length;
        ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());

        for (int k = 0; k < n; k++) {
            // Normalizacja wiersza k
            double pivot = matrix[k][k];
            for (int j = k; j < n; j++) {
                matrix[k][j] /= pivot;
            }
            rhs[k] /= pivot;

            // Eliminacja wierszy poniżej k
            int finalK = k;
            executor.submit(() -> IntStream.range(finalK + 1, n).parallel().forEach(i -> {
                double factor = matrix[i][finalK];
                for (int j = finalK; j < n; j++) {
                    matrix[i][j] -= factor * matrix[finalK][j];
                }
                rhs[i] -= factor * rhs[finalK];
            }));
        }

        executor.shutdown();
        while (!executor.isTerminated()) {
            // Czekanie na zakończenie wszystkich zadań
        }

        // Rozwiązanie układu równań - metoda wsteczna
        double[] solution = new double[n];
        for (int i = n - 1; i >= 0; i--) {
            solution[i] = rhs[i];
            for (int j = i + 1; j < n; j++) {
                solution[i] -= matrix[i][j] * solution[j];
            }
        }
    }

    public static void main(String[] args) {
        int n = 5000 ; // Rozmiar macierzy
        double[][] matrix = generateRandomMatrix(n, n);
        double[] rhs = generateRandomVector(n);

        // Wersja sekwencyjna
        double[][] matrixSeq = copyMatrix(matrix);
        double[] rhsSeq = rhs.clone();
        long startTime = System.currentTimeMillis();
        gaussEliminationSequential(matrixSeq, rhsSeq);
        long endTime = System.currentTimeMillis();
        System.out.println("Czas wykonania (sekwencyjna): " + (endTime - startTime) + " ms");

        // Wersja równoległa
        double[][] matrixPar = copyMatrix(matrix);
        double[] rhsPar = rhs.clone();
        startTime = System.currentTimeMillis();
        gaussEliminationParallel(matrixPar, rhsPar);
        endTime = System.currentTimeMillis();
        System.out.println("Czas wykonania (równoległa): " + (endTime - startTime) + " ms");
    }

    // Funkcja do generowania losowej macierzy
    public static double[][] generateRandomMatrix(int rows, int cols) {
        Random random = new Random();
        double[][] matrix = new double[rows][cols];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                matrix[i][j] = random.nextDouble() * 100;
            }
        }
        return matrix;
    }

    // Funkcja do generowania losowego wektora
    public static double[] generateRandomVector(int size) {
        Random random = new Random();
        double[] vector = new double[size];
        for (int i = 0; i < size; i++) {
            vector[i] = random.nextDouble() * 100;
        }
        return vector;
    }

    // Funkcja do kopiowania macierzy
    public static double[][] copyMatrix(double[][] original) {
        int rows = original.length;
        int cols = original[0].length;
        double[][] copy = new double[rows][cols];
        for (int i = 0; i < rows; i++) {
            System.arraycopy(original[i], 0, copy[i], 0, cols);
        }
        return copy;
    }
}
