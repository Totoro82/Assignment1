#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

double random_double() {
    return (double)rand() / RAND_MAX;
}

int main(int argc, char *argv[]) {
    /// check if arguments are valid
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);

    if (N <= 0) {
        fprintf(stderr, "N must be positive.\n");
        exit(EXIT_FAILURE);
    }

    double *array = malloc(N * sizeof(double)); //dynamiclly allocs space for array
    if (!array) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); //diferent seed for each run
    for (int i = 0; i < N; i++) { // create N random values btw [0,1]
        array[i] = random_double();
    }

    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        free(array);
        exit(EXIT_FAILURE);
    }

    struct timeval start, end;
    gettimeofday(&start, NULL); //start timer

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid1 == 0) { //child 1
        close(pipe1[0]); // close read end (the forked process only writes)
        double sum1 = 0;
        for (int i = 0; i < N / 2; i++) {
            sum1 += array[i];
        }
        write(pipe1[1], &sum1, sizeof(double));
        close(pipe1[1]);
        free(array);
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) { // Child 2
        close(pipe2[0]);
        double sum2 = 0;
        for (int i = N / 2; i < N; i++) {
            sum2 += array[i];
        }
        write(pipe2[1], &sum2, sizeof(double));
        close(pipe2[1]);
        free(array);
        exit(EXIT_SUCCESS);
    }

    //parent
    close(pipe1[1]);
    close(pipe2[1]);

    //read vboth values
    double sum1, sum2;
    read(pipe1[0], &sum1, sizeof(double));
    read(pipe2[0], &sum2, sizeof(double));

    //close writing pipelines
    close(pipe1[0]);
    close(pipe2[0]);

    // wait for end (so we don't have phantom processes)
    wait(NULL);
    wait(NULL);

    double total = sum1 + sum2;

    gettimeofday(&end, NULL); // end timer
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
                     (end.tv_usec - start.tv_usec) / 1000.0;

    printf("Sum = %.6f\n", total);
    printf("Elapsed time = %.3f ms\n", elapsed);

    free(array);//unalloc
    return 0;
}