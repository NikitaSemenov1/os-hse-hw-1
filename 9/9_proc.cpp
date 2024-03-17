#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

const char *input_fifo = "input_fifo";
const char *output_fifo = "output_fifo";

const int BUFFER_SIZE = 3;

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
    while(true) {
        int r = read(input_fd, buf, BUFFER_SIZE);

        if (r == -1) {
            perror("Can't read from input channel");
            exit(1);
        } else if (r == 0) {
            break;
        }

        PerfromAction(buf, r);
        write(output_fd, buf, r);
    }
}

int main() {

    mkfifo(input_fifo, S_IFIFO | 0666);
    mkfifo(output_fifo, S_IFIFO | 0666);

    int input_fd = open(input_fifo, O_RDONLY);
    int output_fd = open(output_fifo, O_WRONLY);


    Processor(input_fd, output_fd);


}