#include "protocol.h"
#include "checksum.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "malloc.h"
#include "string.h"
#include "sys/stat.h"

static int arrcmp(void* arr1, void* arr2, size_t len);
static int serial_blocking_read(int serial_fd, void* buf, size_t len, unsigned int timeout_ms);

static int
arrcmp(void* arr1, void* arr2, size_t len) {
    if (arr1 == NULL | arr2 == NULL) {
        return 1;
    }

    unsigned char* buf1 = (unsigned char*)arr1; 
    unsigned char* buf2 = (unsigned char*)arr2;

    int ret = 0;

    for (size_t i = 0; i < len; i++) {
        if (buf1[i] != buf2[i]) {
            ret++;
        }
    }

    if (ret != 0) {
        return 1;
    }

    return ret;
}

static int
serial_blocking_read(int serial_fd, void* buf, size_t len, unsigned int timeout_ms) {
    if (buf == NULL) {
        return 1;
    }

    unsigned char* msg = (unsigned char*)buf;

    unsigned int timeout_ms_counter = 0;
    while (read(serial_fd, msg, len) != len) {
        if(timeout_ms_counter++ == timeout_ms) {
            return 1;
        } else {
            usleep(1000);
        }
    }
    return 0;
}

int
handshake_send_serial(int serial_fd, unsigned long number_of_packets) {
    int ret = 0;
    unsigned char handshake_str[HANDSHAKE_LENGTH] = {0xDE, 0xAD, 0xBA, 0xBE, 0, 0};

    handshake_str[4] = (number_of_packets >> 24) & 0x000000FF;
    handshake_str[5] = (number_of_packets >> 16) & 0x000000FF;
    handshake_str[6] = (number_of_packets >> 8) & 0x000000FF;
    handshake_str[7] = number_of_packets & 0x000000FF;
    
    ret = checksum_add(handshake_str, HANDSHAKE_LENGTH);
    if (ret != 0) {
        printf("Failed to add checksum to handshake message on send.\n");
        return 1;
    }

    ret = write(serial_fd, handshake_str, HANDSHAKE_LENGTH);
    if (ret != HANDSHAKE_LENGTH) {
        printf("Failed to send handshake message.\nExpected to send %d bytes, but sent %d.\n", ret, HANDSHAKE_LENGTH);
        return 1;
    }

    printf("Succesfully sent handshake message.\n");

    return 0;
}

int
handshake_receive_serial(int serial_fd) {
    int ret = 0;
    unsigned char handshake_str[HANDSHAKE_LENGTH] = {0xFA, 0xDE, 0xFA, 0xCE, 0, 0};

    unsigned char receive_buf[HANDSHAKE_LENGTH];
    ret = serial_blocking_read(serial_fd, receive_buf, HANDSHAKE_LENGTH, 100);
    if (ret != 0) {
        printf("Failed to receive handshake message.\nExpected to receive %d bytes, but received %d.\n", HANDSHAKE_LENGTH, ret);
        return 1;
    }

    ret = (arrcmp(receive_buf, handshake_str, 4) != 0) & (checksum_valid(receive_buf, HANDSHAKE_LENGTH));
    if (ret != 0) {
        printf("Handshake message missmatch.\n");
        return 1;
    }
    
    printf("Succesfully received handshake message.\n");

    return 0;
}

int
ack_receive_serial(int serial_fd) {
    int ret = 0;
    unsigned char ack_str[ACK_LENGTH] = {0xFE, 0xED, 0, 0};

    ret = checksum_add(ack_str, ACK_LENGTH);
    if (ret != 0) {
        printf("Failed to add checksum to acknowledge message on receive.\n");
        return 1;
    }

    unsigned char receive_buf[ACK_LENGTH];
    ret = serial_blocking_read(serial_fd, receive_buf, ACK_LENGTH, 1000);
    if (ret != 0) {
        printf("Failed to receive acknowledge message.\nExpected to receive %d bytes, but received %d.\n", ACK_LENGTH, ret);
        return 1;
    }

    ret = (arrcmp(receive_buf, ack_str, ACK_LENGTH - 2) != 0) | (checksum_valid(receive_buf, ACK_LENGTH) != 0);
    if (ret != 0) {
        printf("Acknowledge message missmatch.\n");
        return 1;
    }

    return 0;
}

int
firmware_upgrade_serial(int serial_fd, int firmware_fd) {
    int ret = 0;
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

    size_t trailing_bytes = 0;
    size_t number_of_packets = firmware_stat.st_size / FIRMWARE_BYTES_LENGTH;

    if ((trailing_bytes = firmware_stat.st_size % FIRMWARE_BYTES_LENGTH) != 0) {
        printf("The firmware will be divided in %lu packets.\n", number_of_packets + 1);
        handshake_send_serial(serial_fd, number_of_packets + 1);
    } else {
        printf("The firmware will be divided in %lu packets.\n", number_of_packets);
        handshake_send_serial(serial_fd, number_of_packets); 
    }

    ret = handshake_receive_serial(serial_fd);
    if (ret != 0) {
        return 1;
    }

    unsigned char msg_buf[TOTAL_MSG_LENGTH] = {0xBA, 0xDD};
    for (size_t current_packet_number = 0; current_packet_number < number_of_packets; current_packet_number++) {
        memcpy(&msg_buf[2], &firmware_bin[current_packet_number * FIRMWARE_BYTES_LENGTH], FIRMWARE_BYTES_LENGTH);
        checksum_add(msg_buf, TOTAL_MSG_LENGTH);

        ret = write(serial_fd, msg_buf, TOTAL_MSG_LENGTH);
        if (ret != TOTAL_MSG_LENGTH) {
            printf("Error sending packet number %lu.\n", current_packet_number);
            return 1;
        }

        ret = ack_receive_serial(serial_fd);
        if (ret != 0) {
            printf("No acknowledge on packet number %lu.\n", current_packet_number);
            return 1;
        }
    }

    if (trailing_bytes != 0) {
        memset(&msg_buf[2], 0x00, TOTAL_MSG_LENGTH - 2);
        memcpy(&msg_buf[2], &firmware_bin[firmware_stat.st_size - trailing_bytes], trailing_bytes);
        checksum_add(msg_buf, TOTAL_MSG_LENGTH);

        ret = write(serial_fd, msg_buf, TOTAL_MSG_LENGTH);
        if (ret != TOTAL_MSG_LENGTH) {
            printf("Error sending packet number %lu.\n", number_of_packets + 1);
            return 1;
        }

        ret = ack_receive_serial(serial_fd);
        if (ret != 0) {
            printf("No acknowledge on packet number %lu.\n", number_of_packets + 1);
            return 1;
        }
    }

    if (ret != 0) {
        printf("Unknown error.\n");
        return 1;
    }

    printf("Succesfully written firmware binary.\n");

    free(firmware_bin);

    return 0;
}
