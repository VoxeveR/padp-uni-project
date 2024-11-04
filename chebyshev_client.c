#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <math.h>
#include <stddef.h>
#include "chebyshev.h"

/* Funkcja do generowania losowej macierzy */
void generate_random_matrix(double* matrix, double* vector, int size) {
    int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i * size + j] = (double)rand() / RAND_MAX * 10.0;
        }
        vector[i] = (double)rand() / RAND_MAX * 10.0;
    }
}

/* Funkcja do sekwencyjnego rozwiązania układu równań (dla porównania) */
double solve_sequential(double* A, double* b, int size) {
    struct timespec start, end;
    double *x, *prev_x, *r;
    double alpha = 0.5;
    double beta = 0.1;
    int max_iter = 1000;
    double eps = 1e-6;
    int iter, i, j;
    double diff, temp;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    x = (double*)calloc((size_t)size, sizeof(double));
    prev_x = (double*)calloc((size_t)size, sizeof(double));
    r = (double*)calloc((size_t)size, sizeof(double));
    
    if (x == NULL || prev_x == NULL || r == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    for (iter = 0; iter < max_iter; iter++) {
        for (i = 0; i < size; i++) {
            r[i] = b[i];
            for (j = 0; j < size; j++) {
                r[i] -= A[i * size + j] * x[j];
            }
        }
        
        for (i = 0; i < size; i++) {
            temp = x[i];
            x[i] = x[i] + alpha * r[i];
            prev_x[i] = temp;
        }
        
        diff = 0.0;
        for (i = 0; i < size; i++) {
            diff += (x[i] - prev_x[i]) * (x[i] - prev_x[i]);
        }
        if (sqrt(diff) < eps) break;
        
        alpha = beta * alpha;
    }
    
    free(x);
    free(prev_x);
    free(r);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

/* Funkcja do testowania dla danego rozmiaru */
void test_size(CLIENT* clnt, int size) {
    matrix_data input;
    result_data *result;
    double sequential_time;
    size_t matrix_size = (size_t)(size * size);
    size_t vector_size = (size_t)size;
    
    /* Alokacja pamięci */
    input.matrix.matrix_val = (double*)malloc(matrix_size * sizeof(double));
    input.vector.vector_val = (double*)malloc(vector_size * sizeof(double));
    
    if (input.matrix.matrix_val == NULL || input.vector.vector_val == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    input.size = size;
    input.matrix.matrix_len = matrix_size;
    input.vector.vector_len = vector_size;
    
    /* Generowanie danych testowych */
    generate_random_matrix(input.matrix.matrix_val, input.vector.vector_val, size);
    
    /* Pomiar czasu dla wersji sekwencyjnej */
    sequential_time = solve_sequential(input.matrix.matrix_val, 
                                     input.vector.vector_val, size);
    
    /* Wywołanie RPC */
    result = solve_system_1(&input, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "RPC call failed");
        exit(1);
    }
    
    printf("\nRozmiar problemu: %d x %d\n", size, size);
    printf("Czas wykonania RPC: %f sekund\n", result->computation_time);
    printf("Czas wykonania sekwencyjnego: %f sekund\n", sequential_time);
    printf("Przyspieszenie: %f\n", sequential_time / result->computation_time);
    
    /* Zwolnienie pamięci */
    free(input.matrix.matrix_val);
    free(input.vector.vector_val);
}

int main(int argc, char *argv[]) {
    CLIENT *clnt;
    int sizes[] = {10, 100, 1000, 10000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int i;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s server_host\n", argv[0]);
        exit(1);
    }
    
    /* Utworzenie połączenia z serwerem */
    clnt = clnt_create(argv[1], CHEBYSHEV_PROG, CHEBYSHEV_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(argv[1]);
        exit(1);
    }
    
    /* Inicjalizacja generatora liczb losowych */
    srand((unsigned int)time(NULL));
    
    /* Testowanie dla różnych rozmiarów */
    for (i = 0; i < num_sizes; i++) {
        test_size(clnt, sizes[i]);
    }
    
    clnt_destroy(clnt);
    return 0;
}