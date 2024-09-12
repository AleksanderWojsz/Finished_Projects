#include <sys/socket.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include "utilities.h"
#include "protconst.h"

uint16_t read_port(char const* string) {
    char* endptr;
    unsigned long port = strtoul(string, &endptr, 10);
    if ((port == ULONG_MAX && errno == ERANGE) || *endptr != 0 || port == 0 || port > UINT16_MAX) {
        fprintf(stderr, "ERROR: %s is not a valid port number\n", string);
        exit(EXIT_FAILURE);
    }
    return (uint16_t)port;
}

/*
 * Jeżeli host jest pustym napisem, to host = INADDR_ANY
 */
struct sockaddr_in get_server_address(char const* host, uint16_t port) {
    struct sockaddr_in send_address;
    if (strlen(host) != 0) {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Pozwala na używanie 'localhost' zamiast np. 127.0.0.1
        struct addrinfo* address_result;
        int errcode = getaddrinfo(host, NULL, &hints, &address_result);
        if (errcode != 0) {
            fprintf(stderr, "ERROR: getaddrinfo: %s", gai_strerror(errcode));
            exit(EXIT_FAILURE);
        }

        send_address.sin_family = AF_INET;
        send_address.sin_addr.s_addr = ((struct sockaddr_in*)(address_result->ai_addr))->sin_addr.s_addr;
        send_address.sin_port = htons(port);

        freeaddrinfo(address_result);
    } else {
        send_address.sin_family = AF_INET;
        send_address.sin_addr.s_addr = htonl(INADDR_ANY);
        send_address.sin_port = htons(port);
    }

    return send_address;
}

ssize_t recvn(int fd, void* vptr, size_t n) {
    ssize_t nleft, nread;
    char* ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = recv(fd, ptr, nleft, 0)) <= 0) {
            return nread;
        }

        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t sendn(int fd, const void* vptr, size_t n) {
    ssize_t nleft, nwritten;
    const char* ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = send(fd, ptr, nleft, 0)) <= 0) {
            return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

int32_t set_recv_timeout(int32_t fd) {
    struct timeval to = { .tv_sec = MAX_WAIT, .tv_usec = 0 };
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
}

uint64_t random_uint64(void) {
    uint64_t result = 0;
    for (int i = 0; i < 64; i++) {
        result = result << 1;
        result += rand() % 2;
    }
    return result;
}

uint64_t min(uint64_t a, uint64_t b) {
    return a < b ? a : b;
}

void exit_on_error(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                   const char* message, uint32_t line) {
    if (check_for_error(result, socket_fd, first_free, second_free, message, line)) {
        exit(EXIT_FAILURE);
    }
}

void exit_on_error_or_zero(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                           const char* message, uint32_t line) {
    if (check_for_error_or_zero(result, socket_fd, first_free, second_free, message, line)) {
        exit(EXIT_FAILURE);
    }
}

void exit_on_null(void* pointer, int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line) {
    if (pointer == NULL) {
        check_for_error_or_zero(0, socket_fd, first_free, second_free, message, line);
        exit(EXIT_FAILURE);
    }
}

void exit_with_error(int32_t socket_fd, void* first_free, void* second_free,
                     const char* message, uint32_t line) {
    check_for_error(-1, socket_fd, first_free, second_free, message, line);
    exit(EXIT_FAILURE);
}

bool check_for_error(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                     const char* message, uint32_t line) {
    if (result < 0) {
        print_error(message, line);
        if (socket_fd != -1) {
            close(socket_fd);
        }
        free(first_free);
        free(second_free);
        return true;
    } else {
        return false;
    }
}

bool check_for_error_or_zero(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                             const char* message, uint32_t line) {
    if (result == 0) {
        fprintf(stderr, "ERROR: Return value is zero. %s. Line %d\n", message, line);
        if (socket_fd != -1) {
            close(socket_fd);
        }
        free(first_free);
        free(second_free);
        return true;
    } else if (check_for_error(result, socket_fd, first_free, second_free, message, line)) {
        return true;
    } else {
        return false;
    }
}

void print_error(const char* message, uint32_t line) {
    if (errno == 0) {
        fprintf(stderr, "ERROR: %s. Line %d\n", message, line);
    } else {
        fprintf(stderr, "ERROR: errno %d - %s. %s. Line %d\n", errno, strerror(errno), message, line);
        errno = 0;
    }
}

bool is_length_correct(uint8_t packet_id, ssize_t datagram_length) {
    switch (packet_id) {
        case CONN_ID:
            return datagram_length == sizeof(CONN_Packet) + PACKET_ID_SIZE;
        case CONACC_ID:
            return datagram_length == sizeof(CONACC_Packet) + PACKET_ID_SIZE;
        case CONRJT_ID:
            return datagram_length == sizeof(CONRJT_Packet) + PACKET_ID_SIZE;
        case DATA_ID:
            return datagram_length > (ssize_t)sizeof(DATA_Packet) - (ssize_t)sizeof(char*) + PACKET_ID_SIZE && // Długość danych ma być >= 1
            datagram_length <= MAX_SIZE + (ssize_t)sizeof(DATA_Packet) - (ssize_t)sizeof(char*) + PACKET_ID_SIZE;
        case ACC_ID:
            return datagram_length == sizeof(ACC_Packet) + PACKET_ID_SIZE;
        case RJT_ID:
            return datagram_length == sizeof(RJT_Packet) + PACKET_ID_SIZE;
        case RCVD_ID:
            return datagram_length == sizeof(RCVD_Packet) + PACKET_ID_SIZE;
        default:
            return false;
    }
}

bool is_CONN_correct(CONN_Packet conn_packet, uint8_t protocol_id) {
    return conn_packet.protocol_id == protocol_id && conn_packet.data_length > 0;
}

bool is_CONRJT_correct(CONRJT_Packet conrjt_packet, uint64_t session_id) {
    return conrjt_packet.session_id == session_id;
}

bool is_CONACC_correct(CONACC_Packet conacc_packet, uint64_t session_id) {
    return conacc_packet.session_id == session_id;
}

bool is_DATA_correct(DATA_Packet data_packet, uint64_t session_id,
                     uint64_t expected_packet_number, uint64_t remaining_to_receive) {
    return session_id == data_packet.session_id &&
           expected_packet_number == data_packet.packet_number &&
           data_packet.data_length <= MAX_SIZE &&
           data_packet.data_length <= remaining_to_receive;
}

bool is_ACC_correct(ACC_Packet acc_packet, uint64_t session_id, uint64_t packet_number) {
    return acc_packet.session_id == session_id && acc_packet.packet_number == packet_number;
}

bool is_RJT_correct(RJT_Packet rjt_packet, uint64_t session_id) {
    return rjt_packet.session_id == session_id;
}

bool is_RCVD_correct(RCVD_Packet rcvd_packet, uint64_t session_id) {
    return rcvd_packet.session_id == session_id;
}

ssize_t send_CONACC_RCVD_tcp(int32_t fd, uint64_t session_id, uint8_t packet_id) {
    CONACC_Packet packet;
    packet.session_id = session_id;

    size_t packet_size = PACKET_ID_SIZE + sizeof(packet);
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(packet));

    ssize_t ret = sendn(fd, buffer, packet_size);
    free(buffer);
    return ret;
}

ssize_t send_RJT_tcp(int32_t fd, uint64_t session_id, uint64_t packet_number) {
    ACC_Packet packet;
    packet.session_id = session_id;
    packet.packet_number = htobe64(packet_number);

    size_t packet_size = PACKET_ID_SIZE + sizeof(packet);
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    uint8_t packet_id = RJT_ID;
    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(packet));

    ssize_t ret = sendn(fd, buffer, packet_size);
    free(buffer);
    return ret;
}

ssize_t send_CONN_tcp(int32_t fd, uint64_t session_id, uint8_t protocol_id, uint64_t data_length) {
    CONN_Packet packet;
    packet.session_id = session_id;
    packet.protocol_id = protocol_id;
    packet.data_length = htobe64(data_length);

    size_t packet_size = PACKET_ID_SIZE + sizeof(packet);
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    uint8_t packet_id = CONN_ID;
    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(packet));

    ssize_t ret = sendn(fd, buffer, packet_size);
    free(buffer);
    return ret;
}

ssize_t send_CONACC_tcp(int32_t fd, uint64_t session_id) {
    return send_CONACC_RCVD_tcp(fd, session_id, CONACC_ID);
}

ssize_t send_RCVD_tcp(int32_t fd, uint64_t session_id) {
    return send_CONACC_RCVD_tcp(fd, session_id, RCVD_ID);
}

ssize_t send_DATA_tcp(int32_t fd, uint64_t session_id, uint64_t packet_number, uint32_t data_length, const char* data) {

    DATA_Packet packet;
    packet.session_id = session_id;
    packet.packet_number = htobe64(packet_number);
    packet.data_length = htobe32(data_length);
    packet.data = NULL;

    size_t packet_size = PACKET_ID_SIZE + DATA_PACKET_SIZE + data_length;
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    uint8_t packet_id = DATA_ID;
    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(DATA_Packet) - sizeof(char*));
    memcpy(buffer + PACKET_ID_SIZE + DATA_PACKET_SIZE, data, data_length);

    ssize_t ret = sendn(fd, buffer, packet_size);
    free(buffer);

    return ret;
}

ssize_t recv_CONN_tcp(int32_t fd, CONN_Packet* packet) {
    uint8_t buffer[sizeof(CONN_Packet)];
    ssize_t ret_val = recvn(fd, buffer, sizeof(CONN_Packet));
    if (ret_val <= 0) {
        return ret_val;
    }

    *packet = unpack_CONN(buffer);

    return ret_val;
}

ssize_t recv_CONACC_tcp(int32_t fd, CONACC_Packet* packet) {
    return recvn(fd, &packet->session_id, sizeof(packet->session_id));
}

ssize_t recv_DATA_tcp(int32_t fd, DATA_Packet* packet) {
    uint8_t buffer[DATA_PACKET_SIZE];
    ssize_t ret_val = recvn(fd, buffer, sizeof(DATA_Packet) - sizeof(char*));
    if (ret_val <= 0) {
        return ret_val;
    }

    memcpy(packet, buffer, sizeof(DATA_Packet) - sizeof(char*));
    packet->packet_number = be64toh(packet->packet_number);
    packet->data_length = be32toh(packet->data_length);

    packet->data = malloc(packet->data_length);
    if (packet->data == NULL) {
        return -1;
    }

    return recvn(fd, packet->data, packet->data_length);
}

ssize_t recv_RJT_tcp(int32_t fd, RJT_Packet* packet) {
    uint8_t buffer[sizeof(RJT_Packet)];
    ssize_t ret_val = recvn(fd, buffer, sizeof(RJT_Packet));
    if (ret_val <= 0) {
        return ret_val;
    }

    *packet = unpack_RJT(buffer);

    return ret_val;
}

ssize_t recv_RCVD_tcp(int32_t fd, RCVD_Packet* packet) {
    return recvn(fd, &packet->session_id, sizeof(packet->session_id));
}

ssize_t recv_packet_id_tcp(int32_t fd, uint8_t* packet_id) {
    return recvn(fd, packet_id, PACKET_ID_SIZE);
}

ssize_t send_CONACC_CONRJT_RCVD_udp(int32_t fd, uint64_t session_id, uint8_t packet_id,
                                    struct sockaddr_in server_address) {
    CONACC_Packet packet;
    packet.session_id = session_id;

    size_t packet_size = PACKET_ID_SIZE + sizeof(CONACC_Packet);
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(packet));

    ssize_t ret = sendto(fd, buffer, packet_size, 0,
                         (struct sockaddr*)&server_address, (socklen_t)sizeof(server_address));
    free(buffer);
    return ret;
}

ssize_t send_ACC_RJT_udp(int32_t fd, uint64_t session_id, uint64_t packet_number, uint8_t packet_id,
                         struct sockaddr_in server_address) {
    ACC_Packet packet;
    packet.session_id = session_id;
    packet.packet_number = htobe64(packet_number);

    size_t packet_size = PACKET_ID_SIZE + sizeof(ACC_Packet);
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(packet));

    ssize_t ret = sendto(fd, buffer, packet_size, 0,
                         (struct sockaddr*)&server_address, (socklen_t)sizeof(server_address));
    free(buffer);
    return ret;
}

ssize_t send_CONN_udp(int32_t fd, uint64_t session_id, uint8_t protocol_id, uint64_t data_length,
                      struct sockaddr_in server_address) {
    CONN_Packet packet;
    packet.session_id = session_id;
    packet.protocol_id = protocol_id;
    packet.data_length = htobe64(data_length);

    size_t packet_size = PACKET_ID_SIZE + sizeof(CONN_Packet);
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    uint8_t packet_id = CONN_ID;
    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(packet));

    ssize_t ret = sendto(fd, buffer, packet_size, 0,
                         (struct sockaddr*)&server_address, (socklen_t)sizeof(server_address));

    free(buffer);
    return ret;
}

ssize_t send_CONACC_udp(int32_t fd, uint64_t session_id, struct sockaddr_in server_address) {
    return send_CONACC_CONRJT_RCVD_udp(fd, session_id, CONACC_ID, server_address);
}

ssize_t send_CONRJT_udp(int32_t fd, uint64_t session_id, struct sockaddr_in server_address) {
    return send_CONACC_CONRJT_RCVD_udp(fd, session_id, CONRJT_ID, server_address);
}

ssize_t send_RCVD_udp(int32_t fd, uint64_t session_id, struct sockaddr_in server_address) {
    return send_CONACC_CONRJT_RCVD_udp(fd, session_id, RCVD_ID, server_address);
}

ssize_t send_DATA_udp(int32_t fd, uint64_t session_id, uint64_t packet_number,
                      uint32_t data_length, const char* data, struct sockaddr_in server_address) {
    DATA_Packet packet;
    packet.session_id = session_id;
    packet.packet_number = htobe64(packet_number);
    packet.data_length = htobe32(data_length);
    packet.data = NULL;
    
    size_t packet_size = PACKET_ID_SIZE + DATA_PACKET_SIZE + data_length;
    uint8_t* buffer = malloc(packet_size);
    if (buffer == NULL) {
        return -1;
    }

    uint8_t packet_id = DATA_ID;
    memcpy(buffer, &packet_id, PACKET_ID_SIZE);
    memcpy(buffer + PACKET_ID_SIZE, &packet, sizeof(DATA_Packet) - sizeof(char*));
    memcpy(buffer + PACKET_ID_SIZE + DATA_PACKET_SIZE, data, data_length);

    ssize_t ret = sendto(fd, buffer, packet_size, 0,
                         (struct sockaddr*)&server_address, (socklen_t)sizeof(server_address));
    free(buffer);
    return ret;
}

ssize_t send_ACC_udp(int32_t fd, uint64_t session_id, uint64_t packet_number, struct sockaddr_in server_address) {
    return send_ACC_RJT_udp(fd, session_id, packet_number, ACC_ID, server_address);
}

ssize_t send_RJT_udp(int32_t fd, uint64_t session_id, uint64_t packet_number, struct sockaddr_in server_address) {
    return send_ACC_RJT_udp(fd, session_id, packet_number, RJT_ID, server_address);
}

ssize_t recv_datagram(int32_t fd, uint8_t* buffer, struct sockaddr_in* receive_address) {
    socklen_t address_length = (socklen_t)sizeof(*receive_address);
    return recvfrom(fd, buffer, LARGEST_POSSIBLE_MESSAGE, 0,
                    (struct sockaddr*)receive_address, &address_length);
}

CONN_Packet unpack_CONN(uint8_t* buffer) {
    CONN_Packet packet;
    memcpy(&packet, buffer, sizeof(packet));
    packet.data_length = be64toh(packet.data_length);
    return packet;
}

CONACC_Packet unpack_CONACC(uint8_t* buffer) {
    CONACC_Packet packet;
    memcpy(&packet, buffer, sizeof(packet));
    return packet;
}

CONRJT_Packet unpack_CONRJT(uint8_t* buffer) {
    CONRJT_Packet packet;
    memcpy(&packet, buffer, sizeof(packet));
    return packet;
}

DATA_Packet unpack_DATA(uint8_t* buffer, bool* allocation_error) {
    DATA_Packet packet;
    memcpy(&packet, buffer, sizeof(DATA_Packet) - sizeof(char*));
    packet.packet_number = be64toh(packet.packet_number);
    packet.data_length = be32toh(packet.data_length);

    packet.data = malloc(packet.data_length);
    if (packet.data == NULL) {
        *allocation_error = true;
        return packet;
    }

    memcpy(packet.data, buffer + DATA_PACKET_SIZE, packet.data_length);

    return packet;
}

ACC_Packet unpack_ACC(uint8_t* buffer) {
    ACC_Packet packet;
    memcpy(&packet, buffer, sizeof(packet));
    packet.packet_number = be64toh(packet.packet_number);
    return packet;
}

RJT_Packet unpack_RJT(uint8_t* buffer) {
    RJT_Packet packet;
    memcpy(&packet, buffer, sizeof(packet));
    packet.packet_number = be64toh(packet.packet_number);
    return packet;
}

RCVD_Packet unpack_RCVD(uint8_t* buffer) {
    RCVD_Packet packet;
    memcpy(&packet, buffer, sizeof(packet));
    return packet;
}

uint8_t unpack_packet_id(uint8_t* buffer) {
    uint8_t packet_id;
    memcpy(&packet_id, buffer, PACKET_ID_SIZE);
    return packet_id;
}

uint64_t unpack_session_id(uint8_t* buffer) {
    uint64_t session_id;
    memcpy(&session_id, buffer + PACKET_ID_SIZE, sizeof(session_id));
    return session_id;
}
