#include "stdio.h"
#include "termios.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"

#include "protocol.h"

int
main(int argc, char **argv) {
    if (argc < 3) {
        printf("Error: not enough arguments passed.\n");
        printf("1st argument must be serial port address.\n");
        printf("2nd argument must be firmware binary location.\n");
        return 1;
    }

    int ret = 0;
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
    
    ret = iamboot_firmware_upgrade_serial(serial_fd, firmware_fd);

    return ret;
}

int8_t iamboot_serial_tx(void *pv_arg, void *buf, uint32_t len, uint32_t timeout_ms)
{
    if (pv_arg == NULL || buf == NULL) 
        return -1;

    int *serial_fd_ptr = (int *)pv_arg;
    uint8_t *msg = (uint8_t *)buf;

    uint32_t timeout_cnt = 0;
    while (write(*serial_fd_ptr, msg, len) != len) {
        if (timeout_cnt == timeout_ms)
            return -2;
        usleep(1000);
        timeout_cnt++;
    }
        
    return 0;
}

int8_t iamboot_serial_rx(void *pv_arg, void *buf, uint32_t len, uint32_t timeout_ms)
{
    if (pv_arg == NULL || buf == NULL) 
        return -1;

    int *serial_fd_ptr = (int *)pv_arg;
    uint8_t *msg = (uint8_t *)buf;

    int ret = 0;
    uint32_t timeout_cnt = 0;
    while ((ret = read(*serial_fd_ptr, msg, len)) != len) {
        if (timeout_cnt == timeout_ms)
            return -2;
        usleep(1000);
        timeout_cnt++;
    }
        
    return 0;
}

