#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mthread.h"
#include <math.h>

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
			printf("%d ", m.a[i][j]);
        }
		printf("\n");
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

    thread_exit(NULL);
}

void *infinite(void *arg) {
    while(running);
    thread_exit(NULL);
}
int main() {
    int i, d[NUM_OF_THREADS][2];

    /* Initialise first matrix */
    scanf("%d %d", &(a.rows), &(a.cols));
    buildMatrix(&a);

    /* Initialise second matrix */
    scanf("%d %d", &(b.rows), &(b.cols));
    buildMatrix(&b);

    /* Set result matrix rows and cols */
    c.rows = a.rows;
    c.cols = b.cols;

    /* Initialise the result matrix */
    initMatrix(&c);

    /* Thread identifiers */
    mthread_t tid[NUM_OF_THREADS], extra;

    thread_init();
    
    for (i = 0; i < NUM_OF_THREADS; i++) {
        /* Calculate starting row thread should start process */
        d[i][0] = ceil(i * (double)(c.rows)/NUM_OF_THREADS);

        /* Calculate ending row thread should start process */
        d[i][1] = ceil((i + 1) * (double)(c.rows)/NUM_OF_THREADS);

        /* Create thread and pass args */
        thread_create( &tid[i], multiplyMatrix, d[i]);
    }

    thread_create( &extra, infinite, NULL);
    /* Wait until all threads complete */
    for(i = 0; i < NUM_OF_THREADS; i++) {
        thread_join(tid[i], NULL);
    }
    sleep(5);
    running = 0;
    thread_join(extra, NULL);
    printMatrix(c);

    /* Free all allocated memory */
    freeMatrix(&a);
    freeMatrix(&b);
    freeMatrix(&c);

    return 0;
}