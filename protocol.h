#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

#define HANDSHAKE_LENGTH            10U
#define ACK_LENGTH                  4U
#define FIRMWARE_BYTES_LENGTH       56U
#define TOTAL_MSG_LENGTH            64U

#ifdef IAMBOOT_LOW_LEVEL
#include "usart.h"

uint8_t handshake_send_serial(usart_t* usart);
uint8_t handshake_receive_serial(usart_t* usart, uint32_t* number_of_packets);
uint8_t ack_send_serial(usart_t* usart);
#elif IAMBOOT_HIGH_LEVEL
int handshake_send_serial(int serial_fd, unsigned long number_of_packets);
int handshake_receive_serial(int serial_fd);
int ack_receive_serial(int serial_fd);
int firmware_upgrade_serial(int serial_fd, int firmware_fd);
#else
#error One must specify for wich platform this library should be built.
#endif

#endif
