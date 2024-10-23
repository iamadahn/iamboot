#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define HANDSHAKE_LENGHT 6U

int handshake_send_serial(int serial_fd);
int handshake_receive_serial(int serial_fd);

#endif