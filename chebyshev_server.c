#include <rpc/rpc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <float.h>
#include "chebyshev.h"

#define MAX_THREADS 4
#define EPSILON 1e-6

typedef struct {
    double *A;
    double *b;
    double *x;
    double *r;  // wektor residuum
    int start;
    int end;
    int size;
    double alpha;  // parametr metody Czebyszewa
    double beta;   // parametr metody Czebyszewa
} thread_data;

// Funkcja do obliczania normy wektora
double calculate_norm(double *vector, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        if (isnan(vector[i]) || isinf(vector[i])) {
            fprintf(stderr, "Błąd: NaN lub INF w wektorze na pozycji %d\n", i);
            return NAN;
        }
        sum += vector[i] * vector[i];
    }
    return sqrt(sum);
}

// Funkcja do obliczania iloczynu macierzy A i wektora x
void *matrix_vector_multiply(void *arg) {
    thread_data *data = (thread_data *)arg;
    
    for (int i = data->start; i < data->end; i++) {
        data->r[i] = 0.0;
        for (int j = 0; j < data->size; j++) {
            data->r[i] += data->A[i * data->size + j] * data->x[j];
        }
        data->r[i] = data->b[i] - data->r[i];  // obliczanie residuum r = b - Ax
    }
    
    pthread_exit(NULL);
}

// Funkcja do aktualizacji wektora x
void *update_solution(void *arg) {
    thread_data *data = (thread_data *)arg;
    
    for (int i = data->start; i < data->end; i++) {
        // Sprawdźmy, czy r[i] nie jest NaN lub INF
        if (isnan(data->r[i]) || isinf(data->r[i])) {
            fprintf(stderr, "Błąd: NaN lub INF w residuum na pozycji %d\n", i);
            pthread_exit(NULL);
        }

        // Aktualizacja rozwiązania
        data->x[i] = data->x[i] + data->alpha * data->r[i];

        // Sprawdzenie wyniku
        if (isnan(data->x[i]) || isinf(data->x[i])) {
            fprintf(stderr, "Błąd: NaN lub INF w rozwiązaniu x[%d]\n", i);
            pthread_exit(NULL);
        }
    }
    
    pthread_exit(NULL);
}


output_data *solve_1_svc(input_data *input, struct svc_req *rqstp) {
    static output_data output;
    struct timeval start, end;
    pthread_t threads[MAX_THREADS];
    thread_data thread_args[MAX_THREADS];

    // Walidacja poprawności alokacji pamięci
    if (input == NULL) {
        fprintf(stderr, "Błąd: Brak danych wejściowych.\n");
        return NULL;
    }


    // Alokacja pamięci dla wektorów
    output.x.x_len = input->size;
    output.x.x_val = (double *)malloc(input->size * sizeof(double));
    double *r = (double *)malloc(input->size * sizeof(double));
    if (output.x.x_val == NULL || r == NULL) {
        fprintf(stderr, "Błąd alokacji pamięci dla wektora rozwiązania lub residuum.\n");
        return NULL;
    }

    // Inicjalizacja wektora x
    for (int i = 0; i < input->size; i++) {
        output.x.x_val[i] = 0.0;
    }

    // Parametry metody Czebyszewa
    double lambda_min = 0.1;  // najmniejsza wartość własna (przykładowa)
    double lambda_max = 2.0;  // największa wartość własna (przykładowa)
    double alpha = 2.0 / (lambda_min + lambda_max);
    double beta = (lambda_max - lambda_min) / (lambda_max + lambda_min);

    double initial_residual = 0.0;
    double current_residual = 0.0;

    gettimeofday(&start, NULL);

    // Główna pętla metody Czebyszewa
    for (int iter = 0; iter < input->max_iter; iter++) {
        // Podział pracy między wątki
        int chunk = input->size / MAX_THREADS;

        // Obliczanie residuum r = b - Ax
        for (int t = 0; t < MAX_THREADS; t++) {
            thread_args[t].A = input->A.A_val;
            thread_args[t].b = input->b.b_val;
            thread_args[t].x = output.x.x_val;
            thread_args[t].r = r;
            thread_args[t].size = input->size;
            thread_args[t].start = t * chunk;
            thread_args[t].end = (t == MAX_THREADS-1) ? input->size : (t+1) * chunk;
            thread_args[t].alpha = alpha;
            thread_args[t].beta = beta;
            
            // Sprawdzamy, czy wskaźniki nie są NULL
            if (thread_args[t].A == NULL || thread_args[t].b == NULL || 
                thread_args[t].x == NULL || thread_args[t].r == NULL) {
                fprintf(stderr, "Błąd: Niepoprawny wskaźnik danych w wątku %d\n", t);
                return NULL;
            }

            pthread_create(&threads[t], NULL, matrix_vector_multiply, &thread_args[t]);
        }

        for (int t = 0; t < MAX_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }

        // Obliczanie normy residuum
        current_residual = calculate_norm(r, input->size);

		if (isnan(current_residual) || isinf(current_residual)) {
            fprintf(stderr, "Błąd: NaN lub INF w residuum po iteracji %d\n", iter);
            break;
        }

        if (iter == 0) {
            initial_residual = current_residual;
            if (initial_residual == 0.0) {
                fprintf(stderr, "Błąd: Początkowe residuum równe 0, brak postępu.\n");
                break;
            }
        }

        // Warunek zakończenia
        if (current_residual / initial_residual < EPSILON) {
            printf("Osiągnięto zbieżność po %d iteracjach\n", iter);
            break;
        }

        // Aktualizacja rozwiązania
        for (int t = 0; t < MAX_THREADS; t++) {
            pthread_create(&threads[t], NULL, update_solution, &thread_args[t]);
        }

        for (int t = 0; t < MAX_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }

    }

    gettimeofday(&end, NULL);
    output.time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // Zapisanie końcowego residuum
    output.residual = current_residual / initial_residual;


    // Zwalnianie pamięci
    free(r);
    
    return &output;
}
