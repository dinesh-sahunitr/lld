#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define BCM2837_GPIO_BASE 0x3F200000
#define GPIO_SET_OFFSET   0x1C  // Set Pins 0–31 to HIGH
#define GPIO_CLR_OFFSET   0x28  // Set Pins 0–31 to LOW
#define GPIO_FSEL2_OFFSET 0x08  // Controls GPIO 20-29
#define GPIO_LEV_OFFSET   0x34  // Offset to read pin levels

static void __iomem *gpio_base;
static struct class *dev_class;
static int major;

// In Hardware Drivers, registers are usually "point-of-access"
// We often ignore the offset because offset 0 is the ONLY valid place.

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off) {
    char kbuf[2] = {0};

    // For hardware registers, we usually ignore the incoming 'off' 
    // and don't update it, or we simply treat every write as offset 0.
    
    if (copy_from_user(kbuf, buffer, min(len, sizeof(kbuf) - 1))) 
        return -EFAULT;

    if (kbuf[0] == '1') {
        writel(1 << 21, gpio_base + GPIO_SET_OFFSET);
    } else if (kbuf[0] == '0') {
        writel(1 << 21, gpio_base + GPIO_CLR_OFFSET);
    }

    // Return the number of bytes written. 
    // Do NOT increment *off if you want the user to be able to 
    // call pwrite at offset 0 repeatedly without seeking.
    return len;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *off) {
    unsigned int val;
    char kbuf[2];

    // We only provide data if they read from the start
    if (*off > 0) return 0; 

    val = readl(gpio_base + GPIO_LEV_OFFSET);
    kbuf[0] = (val & (1 << 21)) ? '1' : '0';
    kbuf[1] = '\n';

    if (copy_to_user(buffer, kbuf, 2)) return -EFAULT;

    *off += 2; 
    return 2;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
};

static int __init gpio_driver_init(void) {
    major = register_chrdev(0, "pi_gpio", &fops);
    
    // Mapping the block
    gpio_base = ioremap(BCM2837_GPIO_BASE, 0xB4);
    
    // Set GPIO 21 as Output using readl/writel
    unsigned int val = readl(gpio_base + GPIO_FSEL2_OFFSET);
    val &= ~(7 << 3); // Clear bits 5,4,3 (GPIO 21)
    val |= (1 << 3);  // Set bit 3 (Value 001 = Output)
    writel(val, gpio_base + GPIO_FSEL2_OFFSET);

    dev_class = class_create("pi_gpio_class");
    device_create(dev_class, NULL, MKDEV(major, 0), NULL, "pi_gpio");
    
    return 0;
}

static void __exit gpio_driver_exit(void) {
    iounmap(gpio_base);
    device_destroy(dev_class, MKDEV(major, 0));
    class_destroy(dev_class);
    unregister_chrdev(major, "pi_gpio");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);
MODULE_LICENSE("GPL");
