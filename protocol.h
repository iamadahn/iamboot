#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define HANDSHAKE_LENGTH            10U
#define ACK_LENGTH                  4U
#define FIRMWARE_BYTES_LENGTH       56U
#define TOTAL_MSG_LENGTH            64U

int handshake_send_serial(int serial_fd, unsigned long number_of_packets);
int handshake_receive_serial(int serial_fd);
int ack_receive_serial(int serial_fd);
int firmware_upgrade_serial(int serial_fd, int firmware_fd);

#endif