#include "czebyszew.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

SolverResult solverResult;
double x[25] = {0};
double r[25];

void calculateResidual(LinearSystem *linearSystem) {
    int size = linearSystem->size;
    for (int i = 0; i < size; i++) {
        r[i] = linearSystem->vector[i];
        for (int j = 0; j < size; j++) {
            r[i] -= linearSystem->matrix[i][j] * solverResult.solutionVector[j];
        }
    }
}

void updateSolutionVector(LinearSystem *linearSystem, double alpha) {
    int size = linearSystem->size;
    for (int i = 0; i < size; i++) {
        solverResult.solutionVector[i] += alpha * r[i];
    }
}

SolverResult *calculatechebyshevsolution_1_svc(LinearSystem *linearSystem, struct svc_req *rqstp)
{
    int size = linearSystem->size;
    double *b = linearSystem->vector;
    double errorTolerance = linearSystem->errorTolerance;
    int maxIterations = linearSystem->maxIterations;

    solverResult.iterationsPerformed = 0;

    double lambdaMin = 0.612, lambdaMax = 5.576;
    double d = (lambdaMax + lambdaMin) / 2;
    double c = (lambdaMax - lambdaMin) / 2;

    int k;
    for (k = 0; k < maxIterations; k++) {
        solverResult.iterationsPerformed = k + 1;

        calculateResidual(linearSystem);

        double normR = 0;
        for (int i = 0; i < size; i++) {
            normR += r[i] * r[i];
        }
        normR = sqrt(normR);
        if (normR < linearSystem->errorTolerance) {
            solverResult.status = 0;
            return &solverResult;
        }

        double alpha = 1.0 / (d + c * cos(3.14159265358979323846 * (k + 0.5) / linearSystem->maxIterations));
        updateSolutionVector(linearSystem, alpha);
    }

    solverResult.status = 1;
    solverResult.iterationsPerformed = maxIterations;
    return &solverResult;
}