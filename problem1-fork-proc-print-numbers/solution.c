#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Программе передается аргумент — целое число N > 0.
// Необходимо создать N-1 дополнительных процессов таким образом,
// чтобы у каждого процесса было не более одного дочернего процесса.
// Каждый из процессов должен вывести ровно одно число так,
// чтобы на выходе получилась строка: 1 2 3 4 ... N
// Между числами — ровно один пробел, строка завершается символом '\n'.

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    if (n <= 0) {
        fprintf(stderr, "N must be a positive integer\n");
        return 1;
    }

    int current = 1;

    while (1) {
        if (current == n) {
            printf("%d\n", current);
            fflush(stdout);
            return 0;
        }

        printf("%d ", current);
        fflush(stdout);

        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("fork");
            return 1;
        }

        if (child_pid == 0) {
            current++;
            continue;
        }

        if (waitpid(child_pid, NULL, 0) == -1) {
            perror("waitpid");
            return 1;
        }

        return 0;
    }
}
