#define DEFAULT_SYMBOL_NAMESPACE DMA_BUF
#include <linux/module.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "npu_mgr"
#define NPU_MAGIC 'N'
#define NPU_IOCTL_GET_BUF_FD _IOR(NPU_MAGIC, 2, int)

static int major;
static struct class *npu_class;
static struct device *npu_dev;

struct npu_buffer {
    void *vaddr;
    dma_addr_t dma_addr;
    size_t size;
};

/* --- 1. DMA-BUF Operations --- */

static int npu_dmabuf_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma) {
    struct npu_buffer *buf = dmabuf->priv;
    /* Use NULL or npu_dev->parent to ensure the mmap logic finds a valid DMA device */
    return dma_mmap_coherent(NULL, vma, buf->vaddr, buf->dma_addr, buf->size);
}

static void npu_dmabuf_release(struct dma_buf *dmabuf) {
    struct npu_buffer *buf = dmabuf->priv;
    pr_info("NPU_MGR: Releasing DMA-BUF memory\n");
    /* Use NULL to match the allocation call */
    dma_free_coherent(NULL, buf->size, buf->vaddr, buf->dma_addr);
    kfree(buf);
}

static struct sg_table *npu_map_dma_buf(struct dma_buf_attachment *attachment, enum dma_data_direction direction) {
    return ERR_PTR(-ENOTSUPP); 
}
static void npu_unmap_dma_buf(struct dma_buf_attachment *attachment, struct sg_table *table, enum dma_data_direction direction) {}

static const struct dma_buf_ops npu_dmabuf_ops = {
    .mmap = npu_dmabuf_mmap,
    .release = npu_dmabuf_release,
    .map_dma_buf = npu_map_dma_buf,
    .unmap_dma_buf = npu_unmap_dma_buf,
};

/* --- 2. IOCTL Handler --- */

static long npu_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == NPU_IOCTL_GET_BUF_FD) {
        struct npu_buffer *buf;
        struct dma_buf *dmabuf;
        struct dma_buf_export_info exp_info = {0};
        int fd;

        buf = kzalloc(sizeof(*buf), GFP_KERNEL);
        if (!buf) return -ENOMEM;
        
        buf->size = 4096;

        /* CHANGED: Passing NULL tells the kernel to use the default DMA pool */
        buf->vaddr = dma_alloc_coherent(NULL, buf->size, &buf->dma_addr, GFP_KERNEL);
        if (!buf->vaddr) { 
            pr_err("NPU_MGR: dma_alloc_coherent failed!\n");
            kfree(buf); 
            return -ENOMEM; 
        }

        snprintf(buf->vaddr, buf->size, "NPU_FIRMWARE_LOG: Initializing Performance Counters...");

        exp_info.ops = &npu_dmabuf_ops;
        exp_info.size = buf->size;
        exp_info.flags = O_RDWR;
        exp_info.priv = buf;
        exp_info.owner = THIS_MODULE;

        dmabuf = dma_buf_export(&exp_info);
        if (IS_ERR(dmabuf)) {
            dma_free_coherent(NULL, buf->size, buf->vaddr, buf->dma_addr);
            kfree(buf);
            return PTR_ERR(dmabuf);
        }

        fd = dma_buf_fd(dmabuf, O_CLOEXEC);
        return copy_to_user((int __user *)arg, &fd, sizeof(fd)) ? -EFAULT : 0;
    }
    return -ENOTTY;
}

static struct file_operations fops = { .owner = THIS_MODULE, .unlocked_ioctl = npu_ioctl };

static int __init npu_exporter_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    npu_class = class_create(DEVICE_NAME);
    npu_dev = device_create(npu_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    
    /* On 6.12 Pi kernels, setting the mask on a virtual device is often ignored, 
       but we keep it here for best practice. */
    dma_set_mask_and_coherent(npu_dev, DMA_BIT_MASK(32));
    
    pr_info("NPU_MGR: Exporter Loaded\n");
    return 0;
}

static void __exit npu_exporter_exit(void) {
    device_destroy(npu_class, MKDEV(major, 0));
    class_destroy(npu_class);
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(npu_exporter_init);
module_exit(npu_exporter_exit);

MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(DMA_BUF);
