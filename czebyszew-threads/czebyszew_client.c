#include "czebyszew.h"
#include <memory.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    CLIENT *client;
    LinearSystem linearSystem;
    SolverResult *solverResult;
    char *serverHost;

	clock_t start, end;
    double cpu_time_used;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_host>\n", argv[0]);
        return 1;
    }
    serverHost = argv[1];

    client = clnt_create(serverHost, CHEBYSHEV_SOLVER_PROG, CHEBYSHEV_SOLVER_VERS, "udp");
    if (client == NULL) {
        clnt_pcreateerror(serverHost);
        return 1;
    }

    linearSystem.size = 3;
    linearSystem.matrix[0][0] = 4; linearSystem.matrix[0][1] = 1; linearSystem.matrix[0][2] = 2;
    linearSystem.matrix[1][0] = 1; linearSystem.matrix[1][1] = 3; linearSystem.matrix[1][2] = -1;
    linearSystem.matrix[2][0] = 2; linearSystem.matrix[2][1] = -1; linearSystem.matrix[2][2] = 3;
    linearSystem.vector[0] = 4; linearSystem.vector[1] = 2; linearSystem.vector[2] = 6;
    linearSystem.errorTolerance = 0.0001;
    linearSystem.maxIterations = 100;

	start = clock();

    solverResult = calculatechebyshevsolution_1(&linearSystem, client);
    if (solverResult == NULL) {
        fprintf(stderr, "Call failed\n");
        clnt_destroy(client);
        return 1;
    }

	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Time CPU: %f s\n", cpu_time_used);

    if (solverResult->status == 0) {
        printf("Solution found in %d iterations:\n", solverResult->iterationsPerformed);
        for (int i = 0; i < linearSystem.size; i++) {
            printf("x[%d] = %f\n", i, solverResult->solutionVector[i]);
        }
    } else {
        printf("Solution not found within maximum iterations.\n");
    }

    clnt_destroy(client);
    return 0;
}