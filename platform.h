#pragma once

#define READ_ONLY 0x01
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11

struct pcd_platform_data {
	int size;
	int permission;
	const char *serial_number;
};
