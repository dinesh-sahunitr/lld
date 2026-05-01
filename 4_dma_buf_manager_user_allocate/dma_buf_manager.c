#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/iosys-map.h> // Add this header

#define DEVICE_NAME "npu_mgr"
#define NPU_MAGIC 'N'
#define NPU_IOCTL_IMPORT_DMABUF _IOW(NPU_MAGIC, 1, int)

static int major;
static struct class *npu_class;
static struct device *npu_dev;

static int handle_import(int fd) {
    struct dma_buf *dmabuf;
    struct iosys_map map; // The new requirement for 6.12
    void *vaddr;
    int ret;

    // 1. Get the dma_buf from the user's File Descriptor
    dmabuf = dma_buf_get(fd);
    if (IS_ERR(dmabuf)) {
        pr_err("NPU_MGR: Invalid dmabuf FD %d\n", fd);
        return PTR_ERR(dmabuf);
    }

    // 2. Map the buffer using the new API
    // Returns 0 on success, fills 'map'
    ret = dma_buf_vmap(dmabuf, &map);
    if (ret) {
        pr_err("NPU_MGR: vmap failed with %d\n", ret);
        dma_buf_put(dmabuf);
        return ret;
    }

    // Extract the raw virtual address from the map
    vaddr = map.vaddr; 

    pr_info("NPU_MGR: Imported FD %d. Content: %s\n", fd, (char *)vaddr);

    // 3. Write back to the buffer
    snprintf(vaddr, 4096, "Kernel says: Received your logs via dmabuf!");

    // 4. Clean up using the map structure
    dma_buf_vunmap(dmabuf, &map);
    dma_buf_put(dmabuf); 
    
    return 0;
}

static long npu_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int fd;

    switch (cmd) {
        case NPU_IOCTL_IMPORT_DMABUF:
            if (copy_from_user(&fd, (int __user *)arg, sizeof(int)))
                return -EFAULT;
            return handle_import(fd);
        default:
            return -ENOTTY;
    }
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = npu_ioctl,
};

static int __init npu_mgr_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    npu_class = class_create(DEVICE_NAME);
    npu_dev = device_create(npu_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    pr_info("NPU_MGR: Driver loaded\n");
    return 0;
}

static void __exit npu_mgr_exit(void) {
    device_destroy(npu_class, MKDEV(major, 0));
    class_destroy(npu_class);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("NPU_MGR: Driver unloaded\n");
}


module_init(npu_mgr_init);
module_exit(npu_mgr_exit);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(DMA_BUF);
