#ifndef UTILITIES_H
#define UTILITIES_H

#define QUEUE_LENGTH 15
#define MAX_SIZE 64000

#define TCP_ID 1
#define UDP_ID 2
#define UDPR_ID 3

#define CONN_ID 1
#define CONACC_ID 2
#define CONRJT_ID 3
#define DATA_ID 4
#define ACC_ID 5
#define RJT_ID 6
#define RCVD_ID 7

#define PACKET_ID_SIZE 1

#define DATA_PACKET_SIZE (sizeof(DATA_Packet) - sizeof(char*))
#define LARGEST_POSSIBLE_MESSAGE (MAX_SIZE + DATA_PACKET_SIZE + PACKET_ID_SIZE)

typedef struct __attribute__((packed)) {
    uint64_t session_id;
    uint8_t protocol_id;
    uint64_t data_length;
} CONN_Packet;

typedef struct __attribute__((packed)) {
    uint64_t session_id;
} CONACC_Packet, CONRJT_Packet, RCVD_Packet;

typedef struct __attribute__((packed)) {
    uint64_t session_id;
    uint64_t packet_number;
    uint32_t data_length;
    char* data;
} DATA_Packet;

typedef struct __attribute__((packed)) {
    uint64_t session_id;
    uint64_t packet_number;
} ACC_Packet, RJT_Packet;

ssize_t send_CONN_tcp(int32_t fd, uint64_t session_id, uint8_t protocol_id, uint64_t data_length);
ssize_t send_CONACC_tcp(int32_t fd, uint64_t session_id);
ssize_t send_RCVD_tcp(int32_t fd, uint64_t session_id);
ssize_t send_DATA_tcp(int32_t fd, uint64_t session_id, uint64_t packet_number, uint32_t data_length, const char* data);
ssize_t send_RJT_tcp(int32_t fd, uint64_t session_id, uint64_t packet_number);

ssize_t send_CONN_udp(int32_t fd, uint64_t session_id,
                      uint8_t protocol_id, uint64_t data_length, struct sockaddr_in server_address);
ssize_t send_CONACC_udp(int32_t fd, uint64_t session_id, struct sockaddr_in server_address);
ssize_t send_CONRJT_udp(int32_t fd, uint64_t session_id, struct sockaddr_in server_address);
ssize_t send_RCVD_udp(int32_t fd, uint64_t session_id, struct sockaddr_in server_address);
ssize_t send_DATA_udp(int32_t fd, uint64_t session_id, uint64_t packet_number,
                      uint32_t data_length, const char* data, struct sockaddr_in server_address);
ssize_t send_ACC_udp(int32_t fd, uint64_t session_id, uint64_t packet_number, struct sockaddr_in server_address);
ssize_t send_RJT_udp(int32_t fd, uint64_t session_id, uint64_t packet_number, struct sockaddr_in server_address);

ssize_t recv_CONN_tcp(int32_t fd, CONN_Packet* packet);
ssize_t recv_CONACC_tcp(int32_t fd, CONACC_Packet* packet);
ssize_t recv_DATA_tcp(int32_t fd, DATA_Packet* packet);
ssize_t recv_RJT_tcp(int32_t fd, RJT_Packet* packet);
ssize_t recv_RCVD_tcp(int32_t fd, RCVD_Packet* packet);
ssize_t recv_packet_id_tcp(int32_t fd, uint8_t* packet_id);

ssize_t recv_datagram(int32_t fd, uint8_t* buffer, struct sockaddr_in* receive_address);
CONN_Packet unpack_CONN(uint8_t* buffer);
CONACC_Packet unpack_CONACC(uint8_t* buffer);
CONRJT_Packet unpack_CONRJT(uint8_t* buffer);
DATA_Packet unpack_DATA(uint8_t* buffer, bool* allocation_error);
ACC_Packet unpack_ACC(uint8_t* buffer);
RJT_Packet unpack_RJT(uint8_t* buffer);
RCVD_Packet unpack_RCVD(uint8_t* buffer);
uint8_t unpack_packet_id(uint8_t* buffer);

bool is_length_correct(uint8_t packet_id, ssize_t datagram_length);
bool is_CONN_correct(CONN_Packet conn_packet, uint8_t protocol_id);
bool is_CONACC_correct(CONACC_Packet conacc_packet, uint64_t session_id);
bool is_CONRJT_correct(CONRJT_Packet conrjt_packet, uint64_t session_id);
bool is_ACC_correct(ACC_Packet acc_packet, uint64_t session_id, uint64_t packet_number);
bool is_RJT_correct(RJT_Packet rjt_packet, uint64_t session_id);
bool is_RCVD_correct(RCVD_Packet rcvd_packet, uint64_t session_id);
bool is_DATA_correct(DATA_Packet data_packet, uint64_t session_id,
                     uint64_t expected_packet_number, uint64_t remaining_to_receive);

void exit_on_null(void* pointer, int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line);
void exit_on_error_or_zero(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line);
void exit_on_error(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line);
void exit_with_error(int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line);
bool check_for_error_or_zero(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line);
bool check_for_error(ssize_t result, int32_t socket_fd, void* first_free, void* second_free,
                  const char* message, uint32_t line);

void print_error(const char* message, uint32_t line);
uint64_t unpack_session_id(uint8_t* buffer);
ssize_t recvn(int fd, void* vptr, size_t n);
ssize_t sendn(int fd, const void* vptr, size_t n);
uint64_t random_uint64(void);
uint64_t min(uint64_t a, uint64_t b);
uint16_t read_port(char const* string);
int32_t set_recv_timeout(int32_t fd);

struct sockaddr_in get_server_address(char const* host, uint16_t port);

#endif // UTILITIES_H
