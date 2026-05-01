#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/dma-heap.h> // Header for DMA-BUF heap IOCTL definitions
#include <string.h>
#include <errno.h>

/* These definitions must match the driver to ensure the IOCTL "handshake" works */
#define NPU_MGR_DEV "/dev/npu_mgr"      // Our custom driver's device node
#define NPU_MAGIC 'N'                   // Unique Magic number for our driver
#define NPU_IOCTL_IMPORT_DMABUF _IOW(NPU_MAGIC, 1, int) // Command to send an 'int' (the FD) to kernel

/* * On Raspberry Pi 6.12, the 'linux,cma' heap provides physically contiguous memory 
 * which is usually what NPUs and hardware accelerators require.
 */
#define DMA_HEAP_DEV "/dev/dma_heap/linux,cma"

int main() {
    int heap_fd, mgr_fd, dmabuf_fd;
    void *ptr;
    const size_t buf_size = 4096; // We'll allocate exactly 1 page of memory

    /* 1. Open the Heap: We open the system's memory allocator (the DMA Heap) */
    heap_fd = open(DMA_HEAP_DEV, O_RDONLY);
    if (heap_fd < 0) {
        /* Fallback: If CMA heap isn't available, try the standard system heap */
        heap_fd = open("/dev/dma_heap/system", O_RDONLY);
    }
    
    /* Open our custom driver so we have a channel to send the memory to the kernel */
    mgr_fd = open(NPU_MGR_DEV, O_RDWR);

    if (heap_fd < 0 || mgr_fd < 0) {
        fprintf(stderr, "Error: Could not open devices. Check if driver is loaded.\n");
        return -1;
    }

    /* * 2. Allocation: We ask the DMA Heap to give us a buffer.
     * .len: Size in bytes.
     * .fd_flags: Tells the kernel we want to be able to Read/Write via the FD.
     */
    struct dma_heap_allocation_data data = {
        .len = buf_size,
        .fd_flags = O_CLOEXEC | O_RDWR,
    };

    /* This IOCTL returns a File Descriptor (FD) that represents the physical memory */
    if (ioctl(heap_fd, DMA_HEAP_IOCTL_ALLOC, &data) < 0) {
        perror("DMA Heap Allocation failed");
        return -1;
    }
    dmabuf_fd = data.fd; // This integer is now our "handle" to the 4KB buffer
    printf("[Userspace] Allocated DMA-BUF, FD: %d\n", dmabuf_fd);

    /* * 3. Memory Mapping: We map the DMA-BUF into our process's virtual address space.
     * MAP_SHARED is critical: it ensures changes we make are visible to the hardware/kernel.
     */
    ptr = mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, dmabuf_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }

    /* Write data directly into the shared silicon—no system calls needed for this write! */
    strcpy((char *)ptr, "NPU_LOG_PAYLOAD: System check 1-2-3");
    printf("[Userspace] Initial message written: %s\n", (char *)ptr);

    /* * 4. Handover: We pass ONLY the integer File Descriptor to the kernel driver.
     * The kernel will use this FD to find the actual physical memory pages.
     */
    printf("[Userspace] Sending FD %d to Driver...\n", dmabuf_fd);
    if (ioctl(mgr_fd, NPU_IOCTL_IMPORT_DMABUF, &dmabuf_fd) < 0) {
        perror("IOCTL to NPU Manager failed");
        return -1;
    }

    /* * 5. The Magic Moment: Because the kernel modified the same physical RAM 
     * that 'ptr' points to, the new data is already here. No read() required.
     */
    printf("[Userspace] Data after Driver processing: %s\n", (char *)ptr);

    /* Clean up: Release the mapping and close all descriptors */
    munmap(ptr, buf_size);
    close(dmabuf_fd); // Closing the FD decrements the buffer's reference count
    close(mgr_fd);
    close(heap_fd);

    return 0;
}
