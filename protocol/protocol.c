#include "protocol.h"
#include "checksum.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"

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

int
handshake_send_serial(int serial_fd) {
    int ret = 0;
    unsigned char handshake_str[HANDSHAKE_LENGHT] = {0xDE, 0xAD, 0xBA, 0xBE, 0, 0};
    
    ret = checksum_add(handshake_str, HANDSHAKE_LENGHT);
    if (ret != 0) {
        printf("Failed to add checksum to handshake message on send.\n");
        return 1;
    }

    ret = write(serial_fd, handshake_str, HANDSHAKE_LENGHT);
    if (ret != HANDSHAKE_LENGHT) {
        printf("Failed to send handshake message.\nExpected to send %d bytes, but sent %d.\n", ret, HANDSHAKE_LENGHT);
        return 1;
    }

    printf("Succesfully sent handshake message.\n");

    return 0;
}

int
handshake_receive_serial(int serial_fd) {
    int ret = 0;
    unsigned char handshake_str[HANDSHAKE_LENGHT] = {0xFA, 0xDE, 0xFA, 0xCE, 0, 0};

    ret = checksum_add(handshake_str, HANDSHAKE_LENGHT);
    if (ret != 0) {
        printf("Failed to add checksum to handshake message on receive.\n");
        return 1;
    }

    unsigned char receive_buf[HANDSHAKE_LENGHT];
    ret = read(serial_fd, receive_buf, HANDSHAKE_LENGHT);
    if (ret != HANDSHAKE_LENGHT) {
        printf("Failed to receive handshake message.\nExpected to receive %d bytes, but received %d.\n", HANDSHAKE_LENGHT, ret);
        return 1;
    }

    ret = arrcmp(receive_buf, handshake_str, HANDSHAKE_LENGHT);

    printf("Succesfully received handshake message.\n");
    return 0;
}
