#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <fcntl.h>

const int BUFFER_SIZE = 5000;

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

    int input_channel[2];
    int output_channel[2];

    pipe(input_channel);
    pipe(output_channel);

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
            Output(argv[2], output_channel[0]);
        }

        // processor process

        Processor(input_channel[0], output_channel[1]);

    } else {
        // input process

        Input(argv[1], input_channel[1]);
    }

    return 0;
}
