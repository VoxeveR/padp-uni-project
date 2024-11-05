/*czebyszew.x */

typedef double DynamicArray[25];

struct LinearSystem {
        int size;
        DynamicArray matrix[25];
        DynamicArray vector;
        double errorTolerance;
        int maxIterations;
};

struct SolverResult {
    DynamicArray solutionVector;
    int iterationsPerformed;
    int status; 
};

program CHEBYSHEV_SOLVER_PROG {
    version CHEBYSHEV_SOLVER_VERS {
        SolverResult calculateChebyshevSolution(LinearSystem) = 1;
    } = 1;
} = 0x31234567;