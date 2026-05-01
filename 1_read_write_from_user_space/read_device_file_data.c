#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/err.h> // For IS_ERR macro

#define DEVICE_NAME "pi_driver"
#define BUF_LEN 80

static int major;
static struct class *dev_class = NULL;
static char msg[BUF_LEN] = "Kernel Space: Data received from the Pi driver!\n";

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset) {
    int msg_len = strlen(msg);
    int bytes_to_copy;

    if (*offset >= msg_len) return 0;

    bytes_to_copy = min(len, (size_t)(msg_len - *offset));

    if (copy_to_user(buffer, msg + *offset, bytes_to_copy)) {
        return -EFAULT;
    }

    *offset += bytes_to_copy;
    return bytes_to_copy;
}

static loff_t pi_driver_llseek(struct file *filp, loff_t offset, int whence) {
    // We pass our buffer length (strlen(msg)) as the 4th argument
    return fixed_size_llseek(filp, offset, whence, strlen(msg));
}

static struct file_operations fops = {
    .read = device_read,
    .llseek = pi_driver_llseek,
};

static int __init pi_driver_init(void) {
    // 1. Register Major Number
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;

    // 2. Create Class (Note: Only one argument for Kernel 6.4+)
    dev_class = class_create("pi_class");
    if (IS_ERR(dev_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(dev_class);
    }

    // 3. Create Device
    if (IS_ERR(device_create(dev_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME))) {
        class_destroy(dev_class);
        unregister_chrdev(major, DEVICE_NAME);
        return -1;
    }

    printk(KERN_INFO "Pi Driver: Registered and /dev/%s created\n", DEVICE_NAME);
    return 0;
}

static void __exit pi_driver_exit(void) {
    device_destroy(dev_class, MKDEV(major, 0));
    class_destroy(dev_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Pi Driver: Unregistered\n");
}

module_init(pi_driver_init);
module_exit(pi_driver_exit);

MODULE_LICENSE("GPL");
