#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "[PCD]%s: " fmt "\n", __func__

struct pcd_platform_data pcd1_data = {
	.size = 512,
	.serial_number = "PCD-XYZ-1",
	.permission = READ_WRITE,
};
struct pcd_platform_data pcd2_data = {
	.size = 1024,
	.serial_number = "PCD-XYZ-2",
	.permission = READ_WRITE,
};

static void pcd_release(struct device *dev) {
	struct pcd_platform_data *data = dev->platform_data;
	pr_info("Device released: %s", data->serial_number);
}

struct platform_device device1 = {
	.name = "pseudo-char-device",
	.id = 0,
	.dev = {
		.platform_data = &pcd1_data,
		.release = pcd_release,
	},
};
struct platform_device device2 = {
	.name = "pseudo-char-device",
	.id = 1,
	.dev = {
		.platform_data = &pcd2_data,
		.release = pcd_release,
	},
};

static int __init pcd_platform_init(void) {
	platform_device_register(&device1);
	platform_device_register(&device2);

	pr_info("Device setup module inserted");
	return 0;
}

static void __exit pcd_platform_exit(void) {
	platform_device_unregister(&device1);
	platform_device_unregister(&device2);

	pr_info("Device setup module removed.");
}

module_init(pcd_platform_init);
module_exit(pcd_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atohs");
MODULE_DESCRIPTION("");
