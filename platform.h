#pragma once

#undef pr_fmt
#define pr_fmt(fmt) "[PCD]%s: " fmt "\n", __func__

#define READ_ONLY 0x01
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11

#define PLATFORM_NAME "pseudo-char-device"

struct pcd_platform_data {
	int size;
	int permission;
	const char *serial_number;
};
