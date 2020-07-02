/**
 * Matrix multiplication using three threads using
 * data parralelism.
 * Input is STDIN
 * Input files are available in ../data
 * Namely 1.txt 2.txt 3.txt
 * Output is on STDOUT
 */

#include "mthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define MCHECK(FCALL)                                                    \
    {                                                                    \
        int result;                                                      \
        if ((result = (FCALL)) != 0) {                                   \
            fprintf(stderr, "FATAL: %s (%s)", strerror(result), #FCALL); \
            exit(-1);                                                    \
        }                                                                \
    }

/* Number of threads to use */
#define NUM_OF_THREADS 3

/* Structure to define a matrix */
typedef struct {
    int **a;
    int rows;
    int cols;
}matrix;

/* Global matrices to allow easier threading */
matrix a, b, c;

int running = 1;

/**
 * Initialise the matrix
 * @param m pointer to matrix
 */
void initMatrix(matrix *m) {
	m->a = (int **)malloc(sizeof(int *) * m->rows);

    for(int i = 0; i < m->rows; i++)
		m->a[i] = (int *)malloc(sizeof(int) * m->cols);

    for(int i = 0; i < m->rows; i++)
		for(int j = 0; j < m->cols; j++)
			m->a[i][j] = 0;
}

/**
 * Build matrix from STDIN
 * Assume matrix is not initialised
 * @param m pointer to matrix
 */
void buildMatrix(matrix *m) {
	initMatrix(m);

	for(int i = 0; i < m->rows; i++) {
		for(int j = 0; j < m->cols; j++) {
			fscanf(stdin, "%d", &(m->a[i][j]));
        }
    }
}

/**
 * Print matrix on STDOUT
 * @param m matrix to be printed
 */
void printMatrix(matrix m) {
	for(int i = 0; i < m.rows; i++) {
		for(int j = 0; j < m.cols; j++) {
			fprintf(stdout, "%d ", m.a[i][j]);
        }
		fprintf(stdout, "\n");
	}
}

/**
 * Frees memory allocated
 * @param m pointer to matrix
 */
void freeMatrix(matrix *m) {
    for(int i = 0; i < m->rows; i++) {
        free(m->a[i]);
    }
    free(m->a);
}

/**
 * Multiply two matrices
 * @param arg an integer array having details about the thread's rows of interest
 */
void * multiplyMatrix(void *arg) {
    int i, j, k, sum;
    int *t = (int *)arg;
    int rowStart = t[0];
    int rowEnd = t[1];

    // printf("Thread %d:\trowStart %d\trowEnd %d\tcols %d\n", t[3], rowStart, rowEnd, cols);

    for(i = rowStart; i < rowEnd; i++) {
        for(j = 0; j < c.cols; j++) {
            sum = 0;
            for(k = 0; k < a.cols; k++) {
                sum += a.a[i][k] * b.a[k][j];
            }
            c.a[i][j] = sum;
        }
    }

    mthread_exit(NULL);
}

int main() {
    int i, d[NUM_OF_THREADS][2];

    /* Initialise first matrix */
    fscanf(stdin, "%d %d", &(a.rows), &(a.cols));
    buildMatrix(&a);

    /* Initialise second matrix */
    fscanf(stdin, "%d %d", &(b.rows), &(b.cols));
    buildMatrix(&b);

    /* Set result matrix rows and cols */
    c.rows = a.rows;
    c.cols = b.cols;

    /* Initialise the result matrix */
    initMatrix(&c);

    /* Thread identifiers */
    mthread_t tid[NUM_OF_THREADS];

    MCHECK(mthread_init());

    for (i = 0; i < NUM_OF_THREADS; i++) {
        /* Calculate starting row thread should start process */
        d[i][0] = ceil(i * (double)(c.rows)/NUM_OF_THREADS);

        /* Calculate ending row thread should start process */
        d[i][1] = ceil((i + 1) * (double)(c.rows)/NUM_OF_THREADS);

        /* Create thread and pass args */
        MCHECK(mthread_create(&tid[i], NULL, multiplyMatrix, d[i]));
    }

    /* Wait until all threads complete */
    for(i = 0; i < NUM_OF_THREADS; i++) {
        MCHECK(mthread_join(tid[i], NULL));
    }

    printMatrix(c);

    /* Free all allocated memory */
    freeMatrix(&a);
    freeMatrix(&b);
    freeMatrix(&c);
    return 0;
}