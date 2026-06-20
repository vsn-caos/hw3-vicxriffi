#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// Программе передаётся два аргумента: CMD1 и CMD2.
// Необходимо запустить два процесса, выполняющих эти команды,
// и перенаправить стандартный поток вывода CMD1 на стандартный поток ввода CMD2.
// Эквивалентно: CMD1 | CMD2
// Родительский процесс должен завершаться самым последним.


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <CMD1> <CMD2>\n", argv[0]);
        return 1;
    }

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t first = fork();

    if (first == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (first == 0) {
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        execlp(argv[1], argv[1], (char *)NULL);

        perror("execlp CMD1");
        _exit(1);
    }

    pid_t second = fork();

    if (second == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        wait_for(first);
        return 1;
    }

    if (second == 0) {
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        execlp(argv[2], argv[2], (char *)NULL);

        perror("execlp CMD2");
        _exit(1);
    }
    close(pipefd[0]);
    close(pipefd[1]);

    int first_status = wait_for(first);
    int second_status = wait_for(second);
    return first_status != 0 ? first_status : second_status;
}
