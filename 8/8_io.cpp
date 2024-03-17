#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

const int BUFFER_SIZE = 5000;

const char *input_fifo = "input_fifo";
const char *output_fifo = "output_fifo";

void IO(const char *input_filename, const char *output_filename, int input_fd, int output_fd) {

    int read_file_fd = open(input_filename, O_RDONLY);

    if (read_file_fd == -1) {
        perror("Can't open read file");
        exit(1);
    }

    char buf[BUFFER_SIZE];

    int r = read(read_file_fd, buf, BUFFER_SIZE);

    if (r == -1) {
        perror("Can't read from input file");
        exit(1);
    }

    if (write(input_fd, buf, r) == -1) {
        perror("Error writing to input channel");
        exit(1);
    };


    int write_file_fd = open(output_filename, O_WRONLY);
    if (write_file_fd == -1) {
        perror("Can't open output file");
        exit(1);
    }
    r = read(output_fd, buf, BUFFER_SIZE);

    if (r == -1) {
        perror("Can't read from output channel");
        exit(1);
    }

    write(write_file_fd, buf, r);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        puts("The program expects two arguments - the filenames of the input and output file respectively");

        return 0;
    }

    mkfifo(input_fifo, S_IFIFO | 0666);
    mkfifo(output_fifo, S_IFIFO | 0666);

    int input_fd = open(input_fifo, O_WRONLY);
    int output_fd = open(output_fifo, O_RDONLY);

    IO(argv[1], argv[2], input_fd, output_fd);

    unlink(input_fifo);
    unlink(output_fifo);
    return 0;
}
