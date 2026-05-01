#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd;
    char buffer[100];
    off_t offset = 14; // Length of "Kernel Space: "

    fd = open("/dev/pi_driver", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    // Move the cursor to the 14th byte
    if (lseek(fd, offset, SEEK_SET) < 0) {
        perror("Failed to seek");
        close(fd);
        return -1;
    }

    // Read from the new offset
    int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Data from driver: %s\n", buffer);
    }

    close(fd);
    return 0;
}
