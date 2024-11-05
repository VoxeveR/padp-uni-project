#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "chebyshev.h"

void print_matrix(double *A, int size, int max_elements) {
    printf("Macierz A (pierwsze %d x %d elementy):\n", max_elements, max_elements);
    for (int i = 0; i < (size < max_elements ? size : max_elements); i++) {
        for (int j = 0; j < (size < max_elements ? size : max_elements); j++) {
            printf("%6.2f ", A[i * size + j]);
        }
        printf("\n");
    }
    if (size > max_elements) printf("...\n");
}

void print_vector(double *v, int size, int max_elements, const char *name) {
    printf("%s (pierwsze %d elementy):\n", name, max_elements);
    for (int i = 0; i < (size < max_elements ? size : max_elements); i++) {
        printf("%6.2f ", v[i]);
    }
    printf("\n");
    if (size > max_elements) printf("...\n");
}

void test_size(CLIENT *clnt, int size, int max_iter) {
    input_data input;
    output_data *output;
    
    printf("\n=== Test dla rozmiaru %d ===\n", size);
    
    input.size = size;
    input.max_iter = max_iter;

    input.A.A_len = size * size;
    input.A.A_val = (double *)malloc(size * size * sizeof(double));
    input.b.b_len = size;
    input.b.b_val = (double *)malloc(size * sizeof(double));

    if (input.A.A_val == NULL || input.b.b_val == NULL) {
        printf("Błąd alokacji pamięci dla rozmiaru %d\n", size);
        exit(1);
    }

    srand(time(NULL));  // inicjalizacja generatora liczb losowych
    for (int i = 0; i < size; i++) {
        input.b.b_val[i] = rand() % 10 + 1;  // losowe wartości wektora b (1-10)
        for (int j = 0; j < size; j++) {
            if (i == j)
                input.A.A_val[i * size + j] = 10.0;  // dominująca przekątna
            else
                input.A.A_val[i * size + j] = (rand() % 100) / 100.0;  // losowe wartości (0-1)
        }
    }

    if (size <= 100) {  // wyświetl pełne dane tylko dla małych układów
        print_matrix(input.A.A_val, size, 5);
        print_vector(input.b.b_val, size, 5, "Wektor b");
    }

    output = solve_1(&input, clnt);
    if (output == NULL) {
        clnt_perror(clnt, "Błąd wywołania RPC");
        free(input.A.A_val);
        free(input.b.b_val);
        return;
    }

    printf("\nWyniki:\n");
    printf("Czas wykonania: %f sekund\n", output->time);
    printf("Końcowe względne residuum: %e\n", output->residual);
    print_vector(output->x.x_val, size, 5, "Rozwiązanie x");

    if (size <= 100) {
        printf("\nWeryfikacja rozwiązania:\n");
        for (int i = 0; i < (size < 5 ? size : 5); i++) {
            double sum = 0.0;
            for (int j = 0; j < size; j++) {
                sum += input.A.A_val[i * size + j] * output->x.x_val[j];
            }
            printf("Równanie %d: %f ≈ %f (różnica: %e)\n", 
                   i, sum, input.b.b_val[i], fabs(sum - input.b.b_val[i]));
        }
        if (size > 5) printf("...\n");
    }

    free(input.A.A_val);
    free(input.b.b_val);
}

int main(int argc, char *argv[]) {
    CLIENT *clnt;
    
    if (argc != 2) {
        printf("Użycie: %s <host>\n", argv[0]);
        exit(1);
    }

    clnt = clnt_create(argv[1], CHEBYSHEV_PROG, CHEBYSHEV_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(argv[1]);
        exit(1);
    }

    test_size(clnt, 10, 1000);
    test_size(clnt, 100, 2000);
    test_size(clnt, 1000, 3000);
    test_size(clnt, 10000, 8000);

    clnt_destroy(clnt);
    return 0;
}
23wqa