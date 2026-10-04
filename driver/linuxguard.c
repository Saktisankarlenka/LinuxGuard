#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define DEVICE_NAME "linuxguard"
#define BUFFER_SIZE 256
#define LINUXGUARD_IOCTL_MAGIC 'L'
#define LINUXGUARD_GET_STATUS _IOR(LINUXGUARD_IOCTL_MAGIC, 1, int)
static long linuxguard_ioctl(struct file *file,
                             unsigned int cmd,
                             unsigned long arg)
{
    int status = 1;

    switch (cmd) {

    case LINUXGUARD_GET_STATUS:

        if (copy_to_user((int __user *)arg,
                         &status,
                         sizeof(status)))
            return -EFAULT;

        pr_info("LinuxGuard: status requested via ioctl\n");

        return 0;

    default:
        return -EINVAL;
    }
}

static dev_t dev_number;
static struct cdev linuxguard_cdev;
static struct class *linuxguard_class;

static char message[BUFFER_SIZE] = "LinuxGuard driver active\n";

/* Open */
static int linuxguard_open(struct inode *inode, struct file *file)
{
    pr_info("LinuxGuard: device opened\n");
    return 0;
}

/* Release */
static int linuxguard_release(struct inode *inode, struct file *file)
{
    pr_info("LinuxGuard: device closed\n");
    return 0;
}

/* Read: Kernel -> User Space */
static ssize_t linuxguard_read(struct file *file,
                               char __user *buffer,
                               size_t length,
                               loff_t *offset)
{
    size_t message_length = strlen(message);

    if (*offset >= message_length)
        return 0;

    if (length > message_length - *offset)
        length = message_length - *offset;

    if (copy_to_user(buffer, message + *offset, length))
        return -EFAULT;

    *offset += length;

    pr_info("LinuxGuard: data read by user space\n");

    return length;
}

/* Write: User Space -> Kernel */
static ssize_t linuxguard_write(struct file *file,
                                const char __user *buffer,
                                size_t length,
                                loff_t *offset)
{
    if (length >= BUFFER_SIZE)
        length = BUFFER_SIZE - 1;

    if (copy_from_user(message, buffer, length))
        return -EFAULT;

    message[length] = '\0';

    pr_info("LinuxGuard: data received from user space: %s\n", message);

    return length;
}

/* File operations */
static const struct file_operations linuxguard_fops = {
    .owner = THIS_MODULE,
    .open = linuxguard_open,
    .release = linuxguard_release,
    .read = linuxguard_read,
    .write = linuxguard_write,
    .unlocked_ioctl = linuxguard_ioctl,
};

/* Driver initialization */
static int __init linuxguard_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);

    if (ret < 0) {
        pr_err("LinuxGuard: failed to allocate device number\n");
        return ret;
    }

    cdev_init(&linuxguard_cdev, &linuxguard_fops);

    ret = cdev_add(&linuxguard_cdev, dev_number, 1);

    if (ret < 0) {
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    linuxguard_class = class_create(DEVICE_NAME);

    if (IS_ERR(linuxguard_class)) {
        cdev_del(&linuxguard_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(linuxguard_class);
    }

    if (IS_ERR(device_create(linuxguard_class,
                             NULL,
                             dev_number,
                             NULL,
                             DEVICE_NAME))) {

        class_destroy(linuxguard_class);
        cdev_del(&linuxguard_cdev);
        unregister_chrdev_region(dev_number, 1);

        return -1;
    }

    pr_info("LinuxGuard: driver loaded successfully\n");

    return 0;
}

/* Driver cleanup */
static void __exit linuxguard_exit(void)
{
    device_destroy(linuxguard_class, dev_number);
    class_destroy(linuxguard_class);
    cdev_del(&linuxguard_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("LinuxGuard: driver unloaded\n");
}

module_init(linuxguard_init);
module_exit(linuxguard_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sakti");
MODULE_DESCRIPTION("LinuxGuard Character Device Driver");
MODULE_VERSION("1.0");
