#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/linuxguard"

#define LINUXGUARD_IOCTL_MAGIC 'L'
#define LINUXGUARD_GET_STATUS _IOR(LINUXGUARD_IOCTL_MAGIC, 1, int)

int main(void)
{
    int fd;
    int status = 0;

    fd = open(DEVICE, O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Device opened successfully.\n");

    if (ioctl(fd, LINUXGUARD_GET_STATUS, &status) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    printf("Driver status: %s\n",
           status ? "ACTIVE" : "INACTIVE");

    close(fd);

    printf("Device closed successfully.\n");

    return 0;
}
