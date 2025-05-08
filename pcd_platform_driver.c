#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include "platform.h"

#define MAX_DEVICES 10

struct device_private_data {
	struct pcd_platform_data *data;
	char *buffer;
};

struct driver_private_data {
	dev_t base_devt;
	struct cdev chrdev;
	struct class *class;
	struct device *devices[MAX_DEVICES];
};

static struct driver_private_data driver_data = {
};

/* デバイスが見つかったときのハンドラ */
static int platform_driver_prove(struct platform_device *pdev) {
	pr_info("%s.%d", pdev->name, pdev->id);

	uint minor = pdev->id;
	struct device *dev = device_create(driver_data.class, NULL, driver_data.base_devt + minor, NULL, "pcd%d", minor);
	if (IS_ERR(dev)) {
		pr_err("Device creation failed");
		return (int)PTR_ERR(dev);
	}
	driver_data.devices[minor] = dev;
	pr_info("Device created: %d:%d", MAJOR(driver_data.base_devt), minor);
	return 0;
}

/* デバイスを削除するときのハンドラ */
static void platform_driver_remove(struct platform_device *pdev) {
	pr_info("%s.%d is removed", pdev->name, pdev->id);

	uint minor = pdev->id;
	device_destroy(driver_data.class, driver_data.base_devt + minor);
	driver_data.devices[minor] = NULL;
	pr_info("Device destroyed: %d:%d", minor, MAJOR(driver_data.base_devt));
}

struct platform_driver pcd_driver = {
	.probe = platform_driver_prove,
	.remove = platform_driver_remove,
	.driver = {
		/* ここの名前で管理するデバイスを検索する */
		.name = PLATFORM_NAME,
	},
};

static int pcd_open(struct inode *inode, struct file *file) {
	pr_info("Device opened");
	uint minor = iminor(inode);
	file->private_data = driver_data.devices[minor];
	return 0;
}

static int pcd_release(struct inode *inode, struct file *file) {
	pr_info("Device released");
	return 0;
}

// static ssize_t pcd_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {}

static ssize_t pcd_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
	char *new_buff = kmalloc(count, GFP_KERNEL);
	if (!new_buff) {
		pr_err("Memory allocation failed");
		return -ENOMEM;
	}
	if (copy_from_user(new_buff, buf, (ulong)count) != 0) {
		pr_err("Data copy failed");
		kfree(new_buff);
		return -EFAULT;
	}
	pr_info("%.*s", (int)count, new_buff);

	kfree(new_buff);
	return (ssize_t)count;
}

struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = pcd_open,
	.release = pcd_release,
	.write = pcd_write,
};

static int __init platform_driver_init(void) {
	/* 固有のキャラクターデバイスの範囲を確保 */
	int r = alloc_chrdev_region(&driver_data.base_devt, 0, MAX_DEVICES, "pcd_region");
	if (r < 0) {
		pr_err("Chrdev region allocation failed");
		return r;
	}
	pr_info("chrdev region created: %d:%d...%d", MAJOR(driver_data.base_devt), MINOR(driver_data.base_devt), MAX_DEVICES - 1);

	/* cdevに登録 */
	cdev_init(&driver_data.chrdev, &dev_fops);
	r = cdev_add(&driver_data.chrdev, driver_data.base_devt, MAX_DEVICES);
	if (r < 0) {
		pr_err("Chrdev add failed");
		goto err1;
	}

	driver_data.class = class_create("pcd_class");
	if (IS_ERR(driver_data.class)) {
		pr_err("Class creation failed");
		goto err2;
	}

	platform_driver_register(&pcd_driver);
	pr_info("Platform driver module loaded.");
	return 0;
err2:
	cdev_del(&driver_data.chrdev);
err1:
	unregister_chrdev_region(driver_data.base_devt, MAX_DEVICES);
	return r;
}

static void __exit platform_driver_exit(void) {

	platform_driver_unregister(&pcd_driver);
	class_destroy(driver_data.class);
	cdev_del(&driver_data.chrdev);
	unregister_chrdev_region(driver_data.base_devt, MAX_DEVICES);

	pr_info("Platform driver module unloaded.");
}

module_init(platform_driver_init);
module_exit(platform_driver_exit);

#if !defined(IGNORE)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atohs");
MODULE_DESCRIPTION("PCD Platform driver module");
#endif


