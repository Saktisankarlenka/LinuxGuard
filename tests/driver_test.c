#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/linuxguard"

int main(void)
{
    int fd;
    char buffer[256];

    fd = open(DEVICE, O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Device opened successfully.\n");

    const char *message = "Hello from user space";

    if (write(fd, message, strlen(message)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("Message sent to kernel: %s\n", message);

    memset(buffer, 0, sizeof(buffer));

    if (read(fd, buffer, sizeof(buffer) - 1) < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("Message received from kernel: %s\n", buffer);

    close(fd);

    printf("Device closed successfully.\n");

    return 0;
}
