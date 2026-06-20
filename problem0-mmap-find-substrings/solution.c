#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Программе передаются два аргумента: имя файла и строка для поиска.
// Необходимо найти все вхождения строки в текстовом файле,
// используя отображение на память с помощью системного вызова mmap.
// На стандартный поток вывода вывести список всех позиций (с нуля),
// где встречается искомая строка, по одной на строку.

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <search_string>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *substring = argv[2];
    size_t substring_length = strlen(substring);

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat file_info;

    if (fstat(fd, &file_info) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = (size_t)file_info.st_size;

    if (file_size == 0 || substring_length == 0 ||
        substring_length > file_size) {
        close(fd);
        return 0;
    }

    const char *data = mmap(
        NULL,
        file_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    for (size_t position = 0;
         position + substring_length <= file_size;
         ++position) {
        if (memcmp(
                data + position,
                substring,
                substring_length
            ) == 0) {
            printf("%zu\n", position);
        }
    }

    if (munmap((void *)data, file_size) == -1) {
        perror("munmap");
        return 1;
    }

    return 0;
}

