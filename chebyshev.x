%#include <sys/types.h>

struct result_data {
    double solution<>;
    double computation_time;
};

struct matrix_data {
    double matrix<>;
    double vector<>;
    int size;
};

program CHEBYSHEV_PROG {
    version CHEBYSHEV_VERS {
        result_data SOLVE_SYSTEM(matrix_data) = 1;
    } = 1;
} = 0x20000001;