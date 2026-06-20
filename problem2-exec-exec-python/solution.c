#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Программе на стандартный поток ввода задается арифметическое выражение
// в синтаксисе языка python3. Необходимо вычислить это выражение и вывести результат.
// Использовать дополнительные процессы запрещено — нужно использовать exec.

int main(void) {
    execlp(
        "python3",
        "python3",
        "-c",
        "import sys; print(eval(sys.stdin.read()))",
        (char *)NULL
    );

    perror("execlp");

    return 0;
}
