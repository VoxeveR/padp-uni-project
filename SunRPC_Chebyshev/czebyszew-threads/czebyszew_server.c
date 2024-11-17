#include "czebyszew.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREADS 4 

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

SolverResult solverResult;
double x[25] = {0};
double r[25];
pthread_mutex_t lock;


typedef struct {
    int start;
    int end;
    double alpha;
    LinearSystem *linearSystem;
} ThreadData;

void* calculateResidual(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start; i < data->end; i++) {
        r[i] = data->linearSystem->vector[i];
        for (int j = 0; j < data->linearSystem->size; j++) {
            r[i] -= data->linearSystem->matrix[i][j] * solverResult.solutionVector[j];
        }
    }
    pthread_exit(0);
}

void* updateSolutionVector(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    pthread_mutex_lock(&lock);
    for (int i = data->start; i < data->end; i++) {
        solverResult.solutionVector[i] += data->alpha * r[i];
    }
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

SolverResult *calculatechebyshevsolution_1_svc(LinearSystem *linearSystem, struct svc_req *rqstp)
{
	int size = linearSystem->size;
    double *b = linearSystem->vector;
    double errorTolerance = linearSystem->errorTolerance;
    int maxIterations = linearSystem->maxIterations;

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    solverResult.iterationsPerformed = 0;

    double lambdaMin = 0.612, lambdaMax = 5.576;
    double d = (lambdaMax + lambdaMin) / 2;
    double c = (lambdaMax - lambdaMin) / 2;

    int k;
    for (k = 0; k < maxIterations; k++) {
        solverResult.iterationsPerformed = k + 1;

        int chunkSize = size / NUM_THREADS;
        for (int i = 0; i < NUM_THREADS; i++) {
            threadData[i].linearSystem = linearSystem;
            threadData[i].start = i * chunkSize;
            threadData[i].end = (i == NUM_THREADS - 1) ? size : (i + 1) * chunkSize;
            pthread_create(&threads[i], NULL, calculateResidual, (void*)&threadData[i]);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

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

        for (int i = 0; i < NUM_THREADS; i++) {
            threadData[i].alpha = alpha;
            pthread_create(&threads[i], NULL, updateSolutionVector, (void*)&threadData[i]);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    solverResult.status = 1;
    solverResult.iterationsPerformed = maxIterations;
    return &solverResult;
}
