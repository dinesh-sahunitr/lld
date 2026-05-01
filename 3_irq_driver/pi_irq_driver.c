#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

// Calculation: base (512) + BCM Pin (17) = 529
#define TARGET_GPIO 529

static int irq_number;
static unsigned long last_interrupt_time = 0;

static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
    unsigned long now = jiffies;

    // Debounce: ignore triggers within 200ms
    if (now - last_interrupt_time < msecs_to_jiffies(200)) {
        return IRQ_HANDLED;
    }
    last_interrupt_time = now;

    printk(KERN_INFO "Pi IRQ: Interrupt caught on BCM 17 (System Pin 529)!\n");
    return IRQ_HANDLED;
}

static int __init irq_driver_init(void) {
    int ret;

    // 1. Request the specific system-mapped GPIO
    ret = gpio_request(TARGET_GPIO, "pi_irq_pin");
    if (ret) {
        printk(KERN_ERR "Pi IRQ: Failed to request GPIO %d (Error %d)\n", TARGET_GPIO, ret);
        return ret;
    }

    gpio_direction_input(TARGET_GPIO);

    // 2. Map to IRQ number
    irq_number = gpio_to_irq(TARGET_GPIO);
    if (irq_number < 0) {
        printk(KERN_ERR "Pi IRQ: Could not get IRQ for GPIO %d\n", TARGET_GPIO);
        gpio_free(TARGET_GPIO);
        return irq_number;
    }

    // 3. Register the handler
    ret = request_irq(irq_number, 
                      gpio_irq_handler, 
                      IRQF_TRIGGER_FALLING, 
                      "pi_irq_handler", 
                      NULL);
    if (ret) {
        printk(KERN_ERR "Pi IRQ: request_irq failed (Error %d)\n", ret);
        gpio_free(TARGET_GPIO);
        return ret;
    }

    printk(KERN_INFO "Pi IRQ: Successfully loaded. BCM 17 is IRQ %d\n", irq_number);
    return 0;
}

static void __exit irq_driver_exit(void) {
    free_irq(irq_number, NULL);
    gpio_free(TARGET_GPIO);
    printk(KERN_INFO "Pi IRQ: Driver unloaded\n");
}

module_init(irq_driver_init);
module_exit(irq_driver_exit);
MODULE_LICENSE("GPL");
