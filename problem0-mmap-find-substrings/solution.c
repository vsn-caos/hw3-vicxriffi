#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file> <substring>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *needle = argv[2];
    size_t needle_len = strlen(needle);

    /*
     * Для пустой строки ничего не выводим.
     */
    if (needle_len == 0) {
        return 0;
    }

    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    /*
     * mmap нельзя применять к пустому файлу.
     * Если искомая строка длиннее файла,
     * вхождений также быть не может.
     */
    if (st.st_size == 0 || (off_t)needle_len > st.st_size) {
        close(fd);
        return 0;
    }

    size_t file_size = (size_t)st.st_size;

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

    for (size_t i = 0; i + needle_len <= file_size; ++i) {
        if (memcmp(data + i, needle, needle_len) == 0) {
            /*
             * Каждая позиция должна выводиться
             * на отдельной строке.
             */
            printf("%zu\n", i);
        }
    }

    if (munmap((void *)data, file_size) == -1) {
        perror("munmap");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
