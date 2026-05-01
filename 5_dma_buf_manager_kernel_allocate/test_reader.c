#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define NPU_IOCTL_GET_BUF_FD _IOR('N', 2, int)

int main() {
    int mgr_fd, dmabuf_fd;
    void *ptr;
    const size_t size = 4096;

    mgr_fd = open("/dev/npu_mgr", O_RDWR);
    if (mgr_fd < 0) { perror("Failed to open /dev/npu_mgr"); return -1; }

    /* 1. Request the FD from the driver */
    if (ioctl(mgr_fd, NPU_IOCTL_GET_BUF_FD, &dmabuf_fd) < 0) {
        perror("IOCTL failed");
        return -1;
    }
    printf("[Userspace] Received DMA-BUF FD: %d from driver\n", dmabuf_fd);

    /* 2. Map the driver's coherent memory */
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dmabuf_fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap failed"); return -1; }

    /* 3. Read the data created by the kernel */
    printf("[Userspace] Content from Driver: %s\n", (char *)ptr);

    /* 4. Write something back to the driver's memory */
    strcpy((char *)ptr, "USER_ACK: Logs received. Resetting counters.");
    printf("[Userspace] Modified data in shared buffer.\n");

    munmap(ptr, size);
    close(dmabuf_fd);
    close(mgr_fd);
    return 0;
}
