#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/pi_gpio"

int main() {
    int fd;
    char cmd;
    char read_buf[3];

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf("--- Pi GPIO Test Console (Fixed) ---\n");
    printf("1: HIGH | 0: LOW | r: READ | q: QUIT\n");

    while (1) {
        printf("\n> ");
        if (scanf(" %c", &cmd) != 1) break;

        if (cmd == 'q') break;

        if (cmd == '1' || cmd == '0') {
            // pwrite: Write 1 byte at offset 0
            if (pwrite(fd, &cmd, 1, 0) < 0) {
                perror("Write failed");
            } else {
                printf("Command sent: %c", cmd);
            }
        } 
        else if (cmd == 'r') {
            // pread: Read 2 bytes starting at offset 0
            int n = pread(fd, read_buf, 2, 0); 
            if (n > 0) {
                read_buf[n] = '\0';
                printf("Silicon Status: %s", read_buf);
            } else {
                printf("Read returned EOF (Check driver offset logic)\n");
            }
        }
    }

    close(fd);
    return 0;
}
