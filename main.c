#include "stdio.h"
#include "string.h"
#include "sys/stat.h"
#include "malloc.h"
#include "termios.h"
#include "fcntl.h"
#include "unistd.h"

#include "protocol.h"

int
main(int argc, char **argv) {
    if (argc < 3) {
        printf("Error: not enough arguments passed.\n");
        printf("1st argument must be serial port address.\n");
        printf("2nd argument must be firmware binary location.\n");
        return 1;
    }

    int ret;
    int serial_fd, firmware_fd;

    serial_fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (serial_fd < 0) {
        printf("Failed to open %s serial port.\n", argv[1]);
        return 1;
    } else {
        printf("Serial port %s was opened succesfully!\n", argv[1]);
    }

    firmware_fd = open(argv[2], O_RDONLY | O_NOCTTY);
    if (serial_fd < 0) {
        printf("Failed to open %s firmware binary.\n", argv[2]);
        return 1;
    } else {
        printf("Firmware binary %s was opened succesfully!\n", argv[2]);
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
    
    /*
    char tx_str[7] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, rx_str[7] = {0, 0, 0, 0, 0, 0, 0};

    while ((ret = read(serial_fd, rx_str, 7)) != 7) {
        ret = write(serial_fd, tx_str, 7);
    }

    printf("Read result - %d.\n", ret);
    printf("Echoed %d %d %d %d %d %d %d.\n", rx_str[0], rx_str[1], rx_str[2], rx_str[3], rx_str[4], rx_str[5], rx_str[6]);

    close(serial_fd);
    */

    struct stat firmware_stat;
    fstat(firmware_fd, &firmware_stat);
    printf("Firmware size - %lu.\n", firmware_stat.st_size);

    unsigned char* firmware_bin = malloc(sizeof(unsigned char) * firmware_stat.st_size);
    if (firmware_bin == NULL) {
        printf("Failed to allocate memory for firmware binary.\n");
        return 1;
    } else {
        printf("Succesfully allocated memory for firmware binary.\n");
    }

    ret = read(firmware_fd, firmware_bin, firmware_stat.st_size);
    if (ret != firmware_stat.st_size) {
        printf("Failed to read firmware binary - %d bytes read.\n", ret);
        free(firmware_bin);
        return 1;
    } else {
        printf("Succesfully read firmware binary - %d bytes read.\n", ret);
    }

    handshake_send_serial(serial_fd);
    ret = handshake_receive_serial(serial_fd);
    while (ret != 0) {
        ret = handshake_receive_serial(serial_fd);
    }

    free(firmware_bin);

    return 0;
}
