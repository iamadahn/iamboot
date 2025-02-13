#pragma GCC push_options
#pragma GCC optimize ("O0")

#include "protocol.h"
#include "checksum.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "malloc.h"
#include "string.h"
#include "sys/stat.h"

static int arrcmp(void* arr1, void* arr2, size_t len);

static int arrcmp(void* arr1, void* arr2, size_t len)
{
    if (arr1 == NULL || arr2 == NULL) {
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

__attribute__((weak)) int8_t iamboot_serial_tx(void *pv_arg, void *buf, uint32_t len, uint32_t timeout_ms)
{
    return -1;
}

__attribute__((weak)) int8_t iamboot_serial_rx(void *pv_arg, void *buf, uint32_t len, uint32_t timeout_ms)
{
    return -1;
}

int8_t iamboot_handshake_serial_tx(void *pv_arg, uint32_t *number_of_packets, uint32_t timeout_ms)
{
    int8_t ret = 0;
    uint8_t handshake_str[HANDSHAKE_LENGTH] = {0xDE, 0xAD, 0xBA, 0xBE, 0, 0};

    handshake_str[4] = (*number_of_packets >> 24) & 0x000000FF;
    handshake_str[5] = (*number_of_packets >> 16) & 0x000000FF;
    handshake_str[6] = (*number_of_packets >> 8) & 0x000000FF;
    handshake_str[7] = *number_of_packets & 0x000000FF;
    
    ret = checksum_add(handshake_str, HANDSHAKE_LENGTH);
    if (ret != 0) {
#ifdef __linux__
        printf("Failed to add checksum to handshake message on send.\n");
#endif
        return 1;
    }

    ret = iamboot_serial_tx(pv_arg, handshake_str, HANDSHAKE_LENGTH, timeout_ms);
    if (ret != 0) {
#ifdef __linux__
        printf("Failed to send handshake message.\nExpected to send %d bytes, but sent %d.\n", ret, HANDSHAKE_LENGTH);
#endif
        return 1;
    }

#ifdef __linux__
    printf("Succesfully sent handshake message.\n");
#endif

    return 0;
}

int8_t iamboot_handshake_serial_rx(void *pv_arg, uint32_t *number_of_packets, uint32_t timeout_ms)
{
    int8_t ret = 0;
    uint8_t handshake_str[HANDSHAKE_LENGTH] = {0xDE, 0xAD, 0xBA, 0xBE, 0, 0};

    uint8_t receive_buf[HANDSHAKE_LENGTH];
    ret = iamboot_serial_rx(pv_arg, receive_buf, HANDSHAKE_LENGTH, timeout_ms);
    if (ret != 0) {
#ifdef __linux__
        printf("Failed to receive handshake message.\nExpected to receive %d bytes, but received %d.\n", HANDSHAKE_LENGTH, ret);
#endif
        return 1;
    }

    ret = (arrcmp(receive_buf, handshake_str, HANDSHAKE_LENGTH - 2) != 0) & (checksum_valid(receive_buf, HANDSHAKE_LENGTH));
    if (ret != 0) {
#ifdef __linux__
        printf("Handshake message missmatch.\n");
#endif
        return 1;
    }
    
#ifdef __linux__
    printf("Succesfully received handshake message.\n");
#endif

    return 0;
}

int8_t iamboot_ack_serial_tx(void *pv_arg)
{
    uint8_t ret = 0;
    uint8_t ack_str[ACK_LENGTH] = {0xFE, 0xED, 0, 0};

    ret = checksum_add(ack_str, ACK_LENGTH);
    if (ret != 0) {
        return 1;
    }

    ret = iamboot_serial_tx(pv_arg, ack_str, ACK_LENGTH, 100);
    if (ret != 0) {
        return 1;
    }

    return 0;
}

int8_t iamboot_ack_serial_rx(void *pv_arg)
{
    int8_t ret = 0;
    uint8_t ack_str[ACK_LENGTH] = {0xFE, 0xED, 0, 0};

    ret = checksum_add(ack_str, ACK_LENGTH);
    if (ret != 0) {
#ifdef __linux__
        printf("Failed to add checksum to acknowledge message on receive.\n");
#endif
        return 1;
    }

    uint8_t receive_buf[ACK_LENGTH];
    ret = iamboot_serial_rx(pv_arg, receive_buf, ACK_LENGTH, 1000);
    if (ret != 0) {
#ifdef __linux__
        printf("Failed to receive acknowledge message.\n");
#endif
        return 1;
    }

    ret = (arrcmp(receive_buf, ack_str, ACK_LENGTH - 2) != 0) | (checksum_valid(receive_buf, ACK_LENGTH) != 0);
    if (ret != 0) {
#ifdef __linux__
        printf("Acknowledge message missmatch.\n");
#endif
        return 1;
    }

    return 0;
}

int8_t firmware_upgrade_serial(void *pv_arg, uint8_t *firmware_bytes)
{
    /*
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
    */

    return 0;
}
