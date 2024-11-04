#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <rpc/rpc.h>
#include <time.h>
#include <math.h>
#include "chebyshev.h"

/* Implementacja metody Czebyszewa */
double* chebyshev_method(double* A, double* b, size_t n) {
    double* x = (double*)calloc(n, sizeof(double));
    double* prev_x = (double*)calloc(n, sizeof(double));
    double* r = (double*)calloc(n, sizeof(double));
    size_t i, j;
    size_t iter;
    
    // Parametry metody Czebyszewa
    double alpha = 0.5;
    double beta = 0.1;
    size_t max_iter = 1000;
    double eps = 1e-6;
    
    // Iteracje metody
    for (iter = 0; iter < max_iter; iter++) {
        // Obliczenie residuum r = b - Ax
        for (i = 0; i < n; i++) {
            r[i] = b[i];
            for (j = 0; j < n; j++) {
                r[i] -= A[i * n + j] * x[j];
            }
        }
        
        // Aktualizacja rozwiązania
        for (i = 0; i < n; i++) {
            double temp = x[i];
            x[i] = x[i] + alpha * r[i];
            prev_x[i] = temp;
        }
        
        // Sprawdzenie zbieżności
        double diff = 0.0;
        for (i = 0; i < n; i++) {
            diff += (x[i] - prev_x[i]) * (x[i] - prev_x[i]);
        }
        if (sqrt(diff) < eps) break;
        
        // Aktualizacja parametru alpha
        alpha = beta * alpha;
    }
    
    free(prev_x);
    free(r);
    return x;
}

result_data* solve_system_1_svc(matrix_data* input, struct svc_req* req) {
    static result_data result;
    clock_t start, end;
    size_t i;
    
    start = clock();
    
    // Konwersja z formatu RPC na tablice
    double* matrix = (double*)malloc(input->size * input->size * sizeof(double));
    double* vector = (double*)malloc(input->size * sizeof(double));
    
    for (i = 0; i < (size_t)(input->size * input->size); i++) {
        matrix[i] = input->matrix.matrix_val[i];
    }
    
    for (i = 0; i < (size_t)input->size; i++) {
        vector[i] = input->vector.vector_val[i];
    }
    
    double* solution = chebyshev_method(matrix, vector, input->size);
    end = clock();
    
    // Przygotowanie wyniku w formacie RPC
    result.solution.solution_val = solution;
    result.solution.solution_len = input->size;
    result.computation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    free(matrix);
    free(vector);
    
    return &result;
}