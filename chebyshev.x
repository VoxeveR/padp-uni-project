struct input_data {
    double A<>;    /* macierz współczynników */
    double b<>;    /* wektor wyrazów wolnych */
    int size;     /* rozmiar układu */
    int max_iter; /* maksymalna liczba iteracji */
};

struct output_data {
    double x<>;    /* wektor rozwiązań */
    double time;  /* czas wykonania */
};

program CHEBYSHEV_PROG {
    version CHEBYSHEV_VERS {
        output_data SOLVE(input_data) = 1;
    } = 1;
} = 0x20000001;