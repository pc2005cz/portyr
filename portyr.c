#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef DIRECT_IO
#	include <sys/io.h>
#endif


int main(int argc, char *argv[])
{
	int ret;

	if ((argc <= 1) || (argc >= 5)) {
		fprintf(stderr, "usage:\n");
		fprintf(stderr, "	%s <PORT> [8|16|32] [write val]\n", argv[0]);
		return -1;
	}

	uint16_t addr = strtoul(argv[1], NULL, 0);	//x86 ports only to 0xffff

	bool is_write = false;

	unsigned size = sizeof(uint8_t); //default if only port
	if (argc >= 3) {
		if (!strncmp(argv[2], "8", 1)) {
			size = sizeof(uint8_t);
		} else if (!strncmp(argv[2], "16", 2)) {
			size = sizeof(uint16_t);
		} else if (!strncmp(argv[2], "32", 2)) {
			size = sizeof(uint32_t);
		} else {
			//invalid size, default to uint8_t
			size = sizeof(uint8_t);
		}
	}

	uint8_t val[size];

	if (argc == 4) {
		is_write = true;

		unsigned long int tmp = strtoul(argv[3], NULL, 0);

		switch (size) {
			case sizeof(uint8_t):
			default:
				*((uint8_t *)val) = (uint8_t)tmp;
				break;
			case sizeof(uint16_t):
				*((uint16_t *)val) = (uint16_t)tmp;
				break;
			case sizeof(uint32_t):
				*((uint32_t *)val) = (uint32_t)tmp;
				break;
		}
	}

#ifdef DIRECT_IO

	if (ioperm(addr, size, 1)) {
		perror("ioperm set: ");
		return -1;
	}

	if (is_write) {
		switch (size) {
			case sizeof(uint8_t):
			default:
				outb(*((uint8_t *)val), addr);
				break;
			case sizeof(uint16_t):
				outw(*((uint16_t *)val), addr);
				break;
			case sizeof(uint32_t):
				outl(*((uint32_t *)val), addr);
				break;
		}
	} else {
		switch (size) {
			case sizeof(uint8_t):
			default:
				printf("0x%" PRIx8 "\n", inb(addr));
				break;
			case sizeof(uint16_t):
				printf("0x%" PRIx16 "\n", inw(addr));
				break;
			case sizeof(uint32_t):
				printf("0x%" PRIx32 "\n", inl(addr));
				break;
		}
	}

	if (ioperm(addr, size, 0)) {
		perror("ioperm unset: ");
		return -1;
	}

#else

	int fd_port = open("/dev/port", O_RDWR|O_SYNC);
	if (fd_port < 0) {
		perror("Opening port file (/dev/port): ");
		return -1;
	}

	ret = lseek(fd_port, addr, SEEK_SET);	//addr IO
	if (ret < 0) {
		perror("Setting (seek) to IO port: ");
		close(fd_port);
		return -1;
	}

	if (is_write) {
		ret = write(fd_port, &val , size);
		if (ret < 0) {
			perror("Writing IO port: ");
			close(fd_port);
			return -1;
		}
	} else {
		ret = read(fd_port, &val , size);
		if (ret < 0) {
			perror("Read IO port: ");
			close(fd_port);
			return -1;
		}

		switch (size) {
			case sizeof(uint8_t):
			default:
				printf("0x%" PRIx8 "\n", *((uint8_t *)val));
				break;
			case sizeof(uint16_t):
				printf("0x%" PRIx16 "\n", *((uint16_t *)val));
				break;
			case sizeof(uint32_t):
				printf("0x%" PRIx32 "\n", *((uint32_t *)val));
				break;
		}
	}

	close(fd_port);
#endif

	return 0;
}
