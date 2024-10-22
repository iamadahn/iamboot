#include "stdio.h"
#include "string.h"

#include "termios.h"
#include "fcntl.h"
#include "unistd.h"
#include <strings.h>

int
main(int argc, char **argv) {
    if (argc < 2) {
        printf("Error: no arguments passed.\n");
        return 1;
    }

    int ret;
    int serial_fd = open(argv[1], O_RDWR | O_NOCTTY);


    if (serial_fd < 0) {
        printf("Failed to open %s.\n", argv[1]);
        return 1;
    } else {
        printf("%s was opened succesfully!\n", argv[1]);
    }

    struct termios serial_config;
    bzero(&serial_config, sizeof(serial_config));

    serial_config.c_cflag |= B115200;
    serial_config.c_cflag &= ~PARENB;
    serial_config.c_cflag &= ~CSTOPB;
    serial_config.c_cflag &= ~CSIZE;
    serial_config.c_cflag |= CS8;

    serial_config.c_cflag &= ~CRTSCTS;
    serial_config.c_cflag |= CREAD | CLOCAL;

    serial_config.c_iflag &= ~(IXON | IXOFF | IXANY);
    serial_config.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    serial_config.c_oflag &= ~OPOST;

    if (tcsetattr(serial_fd, TCSANOW, &serial_config) != 0) {
	    printf("Error configuring %s\n", argv[1]);
	    close(serial_fd);
    }

    tcflush(serial_fd, TCIFLUSH);

    char tx_str[7] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, rx_str[7] = {0, 0, 0, 0, 0, 0, 0};

    while ((ret = read(serial_fd, rx_str, 7)) != 7) {
        ret = write(serial_fd, tx_str, 7);
    }

    printf("Read result - %d.\n", ret);
    printf("Echoed %d %d %d %d %d %d %d.\n", rx_str[0], rx_str[1], rx_str[2], rx_str[3], rx_str[4], rx_str[5], rx_str[6]);

    close(serial_fd);

    return 0;
}
