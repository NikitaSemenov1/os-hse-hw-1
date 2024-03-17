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

const char *input_fifo = "io_fifo";

void PerfromAction(char *buf, int len) {
    for (char *c = buf; c != buf + len; c++) {

        if (isupper(*c)) {
            *c = tolower(*c);
        } else if (islower(*c)) {
            *c = toupper(*c);
        }
    }
}

int io_fd;
int io_pid;

void Processor() {

    char buf[BUFFER_SIZE];

    int r = read(io_fd, buf, BUFFER_SIZE);

    if (r == -1) {
        perror( "Can't read from input channel");
        exit(1);
    }

    PerfromAction(buf, r);

    write(io_fd, buf, r);

    if (kill(io_pid, SIGUSR1) != 0) {
        perror("kill");
        exit(1);
    }
}

void Usr1Handle(int) {
    // dummy, just to invoke sigwait
}

void IO(const char *input_filename, const char *output_filename) {

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

    if (write(io_fd, buf, strlen(buf)) == -1) {
        perror("Error writing to input channel");
        exit(1);
    };

    sigset_t sigset;
    sigemptyset(&sigset);

    sigaddset(&sigset, SIGUSR1);
    int sig;
    if (sigwait(&sigset, &sig) != 0) {
        perror("sigwait");
        exit(1);
    }

    int write_file_fd = open(output_filename, O_WRONLY);
    if (write_file_fd == -1) {
        perror("Can't open output file");
        exit(1);
    }
    r = read(io_fd, buf, BUFFER_SIZE);

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

    signal(SIGUSR1, Usr1Handle);

    io_fd = open(input_fifo, O_RDWR);

    if (int io_fork_ret = fork()) {
        if (io_fork_ret == -1) {

            perror("error creating IO process\n");

            strerror(errno);
            exit(1);
        }

        // processor process

        io_pid = io_fork_ret;

        Processor();

        wait(nullptr);
    } else {
        // IO process

        IO(argv[1], argv[2]);
    }

    return 0;
}
