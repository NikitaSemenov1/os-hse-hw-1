#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

const int BUFFER_SIZE = 5000;

const char *input_fifo = "input_fifo";
const char *output_fifo = "output_fifo";

void PerfromAction(char *buf, int len) {
    for (char *c = buf; c != buf + len; c++) {

        if (isupper(*c)) {
            *c = tolower(*c);
        } else if (islower(*c)) {
            *c = toupper(*c);
        }
    }
}

void Processor(int input_fd, int output_fd) {

    char buf[BUFFER_SIZE];

    int r = read(input_fd, buf, BUFFER_SIZE);

    if (r == -1) {
        perror( "Can't read from input channel");
        exit(1);
    }

    PerfromAction(buf, r);

    write(output_fd, buf, r);
}

void Input(const char *filename, int input_fd) {

    int read_file_fd = open(filename, O_RDONLY);

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
}

void Output(const char *filename, int output_fd) {
    int write_file_fd = open(filename, O_WRONLY);
    if (write_file_fd == -1) {
        perror("Can't open output file");
        exit(1);
    }

    char buf[BUFFER_SIZE];
    int r = read(output_fd, buf, BUFFER_SIZE);

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


    if (int input_fork_ret = fork()) {
        if (input_fork_ret == -1) {

            perror("error creating Input process\n");

            strerror(errno);
            exit(1);
        }

        if (int output_fork_ret = fork()) {
            if (output_fork_ret == -1) {
                perror("error creating Output process\n");

                strerror(errno);
                exit(1);
            }
        } else {
            // output process

            int output_fd = open(output_fifo, O_RDONLY);

            Output(argv[2], output_fd);

            return 0;
        }

        // processor process

        int input_fd = open(input_fifo, O_RDONLY);
        int output_fd = open(output_fifo, O_WRONLY);

        Processor(input_fd, output_fd);

    } else {
        // input process

        int input_fd = open(input_fifo, O_WRONLY);

        Input(argv[1], input_fd);

        return 0;
    }

    unlink(input_fifo);
    unlink(output_fifo);

    return 0;
}
