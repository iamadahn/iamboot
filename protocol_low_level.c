#include "protocol.h"
#include "checksum.h"

static int arrcmp(void* arr1, void* arr2, size_t len);

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

uint8_t
handshake_send_serial(usart_t* usart) {
    uint8_t ret = 0;
    uint8_t handshake_str[HANDSHAKE_LENGTH] = {0xFA, 0xDE, 0xFA, 0xCE, 0, 0};
    
    ret = checksum_add(handshake_str, HANDSHAKE_LENGTH);
    if (ret != 0) {
        return 1;
    }

    ret = usart_blck_transmit(usart, handshake_str, HANDSHAKE_LENGTH, 100);
    if (ret != 0) {
        return 1;
    }

    return 0;
}

uint8_t
handshake_receive_serial(usart_t* usart, uint32_t* number_of_packets) {
    uint8_t ret = 0;
    uint8_t handshake_header[4] = {0xDE, 0xAD, 0xBA, 0xBE};

    uint8_t receive_buf[HANDSHAKE_LENGTH];
    ret = usart_blck_receive(usart, receive_buf, HANDSHAKE_LENGTH, 100);
    if (ret != 0) {
        return 1;
    }

    ret = (arrcmp(receive_buf, handshake_header, 4) != 0) & (checksum_valid(receive_buf, HANDSHAKE_LENGTH) != 0);
    if (ret != 0) {
        return 1;
    }

    *number_of_packets = (receive_buf[4] << 24) | (receive_buf[5] << 16) | (receive_buf[6] << 8) | receive_buf[7];

    return 0;
}

uint8_t
ack_send_serial(usart_t* usart) {
    uint8_t ret = 0;
    uint8_t ack_str[ACK_LENGTH] = {0xFE, 0xED, 0, 0};

    ret = checksum_add(ack_str, ACK_LENGTH);
    if (ret != 0) {
        return 1;
    }

    ret = usart_blck_transmit(usart, ack_str, ACK_LENGTH, 100);
    if (ret != 0) {
        return 1;
    }

    return 0;
}

