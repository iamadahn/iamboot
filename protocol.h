#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

#define HANDSHAKE_LENGTH            10U
#define ACK_LENGTH                  4U
#define FIRMWARE_BYTES_LENGTH       56U
#define TOTAL_MSG_LENGTH            64U

int8_t iamboot_serial_tx(void *pv_arg, void *buf, uint32_t len, uint32_t timeout_ms);
int8_t iamboot_serial_rx(void *pv_arg, void *buf, uint32_t len, uint32_t timeout_ms);
int8_t iamboot_handshake_serial_tx(void *pv_arg, uint32_t* number_of_packets, uint32_t timeout_ms);
int8_t iamboot_handshake_serial_rx(void *pv_arg, uint32_t* number_of_packets, uint32_t timeout_ms);
int8_t iamboot_ack_serial_tx(void *pv_arg);
int8_t iamboot_ack_serial_rx(void *pv_arg);

#ifdef __linux__
int8_t iamboot_firmware_upgrade_serial(int serial_fd, int firmware_fd);
#endif

#endif
