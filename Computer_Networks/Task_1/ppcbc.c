/*
 * Schemat wysyłania pakietu w protokole TCP:
 * 1. Skorzystać z funkcji send_...(), np. send_CONN_tcp()
 *
 * Schemat odbierania pakietu w protokole TCP:
 * 1. Odebrać id pakietu funkcją recv_packet_id()
 * 2. Odebrać resztę pakietu odpowiadającą funkcją recv_...(), np. recv_CONACC()
 *
 * Schemat wysyłania pakietu w protokole UDP:
 * 1. Skorzystać z funkcji send_...(), np. send_CONN_udp()
 *
 * Schemat odbierania pakietu w protokole UDP:
 * 1. Odebrać cały datagram funkcją recv_datagram()
 * 2. Odczytać id pakietu funkcją unpack_packet_id()
 * 3. Odczytać resztę pakietu funkcją unpack_...(), np. unpack_CONACC()
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <bits/signum-generic.h>
#include "utilities.h"
#include "protconst.h"

#define GOT_PACKET_RECEIVED_EARLIER 3
#define TIMEOUT 2
#define SUCCESS 1
#define ERROR (-1)

/*
 * Dzieli dane 'data' na pakiety DATA i wysyła je za pomocą protokołu tcp.
 *
 * Zwraca ERROR w przypadku błędu i SUCCESS w przeciwnym przypadku.
 */
int32_t send_DATA_packets_tcp(int32_t socket_fd, uint64_t session_id, int64_t remaining_to_send, const char* data) {
    uint32_t already_sent = 0;
    uint64_t packet_number = 0;
    while (remaining_to_send > 0) {
        uint32_t to_send = min(remaining_to_send, MAX_SIZE);

        if (send_DATA_tcp(socket_fd, session_id, packet_number, to_send, data + already_sent) <= 0) {
            return ERROR;
        }

        packet_number++;
        remaining_to_send -= to_send;
        already_sent += to_send;
    }

    return SUCCESS;
}

/*
 * Oczekuje na pakiet CONACC i sprawdza jego poprawność.
 */
void wait_for_CONACC_tcp(int32_t socket_fd, uint64_t session_id, char* buffer_to_free) {
    uint8_t packet_id;
    CONACC_Packet conacc_packet;

    exit_on_error_or_zero(recv_packet_id_tcp(socket_fd, &packet_id),
                          socket_fd, buffer_to_free, NULL, "While waiting for CONACC. Function recv()", __LINE__);
    if (packet_id != CONACC_ID) {
        exit_with_error(socket_fd, buffer_to_free, NULL, "Got unexpected packet. Expected CONACC", __LINE__);
    }
    exit_on_error_or_zero(recv_CONACC_tcp(socket_fd, &conacc_packet),
                          socket_fd, buffer_to_free, NULL, "While waiting for CONACC. Function recv_CONACC()", __LINE__);
    if (!is_CONACC_correct(conacc_packet, session_id)) {
        exit_with_error(socket_fd, buffer_to_free, NULL, "Received incorrect CONACC", __LINE__);
    }
}

/*
 * Oczekuje na pakiet RCVD/RJT i sprawdza jego poprawność.
 */
void wait_for_RCVD_tcp(int32_t socket_fd, uint64_t session_id) {
    uint8_t packet_id;

    exit_on_error_or_zero(recv_packet_id_tcp(socket_fd, &packet_id),
                          socket_fd, NULL, NULL, "While waiting for RCVD. Function recv()", __LINE__);
    if (packet_id == RCVD_ID) {
        RCVD_Packet rcvd_packet;
        exit_on_error_or_zero(recv_RCVD_tcp(socket_fd, &rcvd_packet),
                              socket_fd, NULL, NULL, "While waiting for RCVD. Function recv_RCVD()", __LINE__);
        if (!is_RCVD_correct(rcvd_packet, session_id)) {
            exit_with_error(socket_fd, NULL, NULL, "Received incorrect RCVD", __LINE__);
        }
    } else if (packet_id == RJT_ID) {
        RJT_Packet rjt_packet;
        exit_on_error_or_zero(recv_RJT_tcp(socket_fd, &rjt_packet),
                              socket_fd, NULL, NULL, "While waiting for RJT. Function recv_RJT()", __LINE__);
        if (!is_RJT_correct(rjt_packet, session_id)) {
            exit_with_error(socket_fd, NULL, NULL, "Received incorrect RJT", __LINE__);
        }
        exit_with_error(socket_fd, NULL, NULL, "Received RJT", __LINE__);
    } else {
        exit_with_error(socket_fd, NULL, NULL, "Got unexpected packet. Expected RCVD/RJT", __LINE__);
    }
}

/*
 * Nawiązuje połączenie z serwerem za pomocą protokołu tcp.
 * Pobiera dane z stdin i przeprowadza pełną komunikację z serwerem.
 */
void tcp(struct sockaddr_in server_address, uint64_t session_id) {

    // Czytanie z stdin
    char* line = NULL;
    size_t len = 0;
    int64_t remaining_to_send = getdelim(&line, &len, EOF, stdin);

    // Z dokumentacji: "Buffer (line) should be freed even if getdelim() failed"
    exit_on_error(remaining_to_send, -1, line, NULL, "Function getdelim()", __LINE__);

    int32_t socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    exit_on_error(socket_fd, socket_fd, NULL, NULL, "Function socket()", __LINE__);

    exit_on_error(set_recv_timeout(socket_fd), socket_fd, NULL, NULL, "Function set_recv_timeout()", __LINE__);

    exit_on_error(connect(socket_fd, (const struct sockaddr*)&server_address, sizeof(server_address)),
                  socket_fd, NULL, NULL, "Function connect()", __LINE__);

    // Wysyłanie pakietu CONN
    exit_on_error_or_zero(send_CONN_tcp(socket_fd, session_id, TCP_ID, remaining_to_send),
                          socket_fd, line, NULL, "Function send_CONN_tcp()", __LINE__);

    // Czekanie na CONACC
    wait_for_CONACC_tcp(socket_fd, session_id, line);

    // Wysyłanie DATA
    exit_on_error(send_DATA_packets_tcp(socket_fd, session_id, remaining_to_send, line),
                  socket_fd, line, NULL, "Function send_DATA_packets_tcp()", __LINE__);

    free(line);

    // Czekanie na RCVD/RJT
    wait_for_RCVD_tcp(socket_fd, session_id);

    close(socket_fd);
}

/*
 * Oczekuje na ACC lub RJT i sprawdza ich poprawność.
 *
 * Jeśli otrzymany pakiet ACC/RJT jest niepoprawny, to kończy program z niezerowym kodem wyjścia.
 * Jeśli w czasie 'MAX_WAIT' nie otrzyma żadnego pakietu, to zwraca 'TIMEOUT'
 *      lub kończy program z niezerowym kodem wyjścia, jeśli limit prób został osiągnięty.
 * Jeśli otrzyma ACC z wcześniejszym numerem pakietu lub CONACC, to zwraca 'GOT_PACKET_RECEIVED_EARLIER'.
 * W przeciwnym przypadku zwraca 'SUCCESS'.
 */
int32_t wait_for_ACC_udp(int32_t socket_fd, uint64_t session_id, char* line, uint8_t* buffer,
                         struct sockaddr_in server_address, uint32_t attempt, uint64_t expected_packet_number) {

    ssize_t datagram_length = recv_datagram(socket_fd, buffer, &server_address);
    if (datagram_length <= 0) {
        if (errno == EAGAIN) { // Timeout
            if (attempt == 1 + MAX_RETRANSMITS) { // +1, bo domyślnie jest jedna próba i ileś retransmisji
                exit_with_error(socket_fd, buffer, line, "Reached limit of DATA retransmissions", __LINE__);
            } else {
                print_error("Timeout while waiting for ACC/RJT. Retransmitting DATA", __LINE__);
                return TIMEOUT;
            }
        } else {
            exit_with_error(socket_fd, buffer, line, "While waiting for ACC/RJT. Function recv_datagram()", __LINE__);
        }
    }

    uint8_t packet_id = unpack_packet_id(buffer);
    if (!is_length_correct(packet_id, datagram_length)) {
        exit_with_error(socket_fd, buffer, line,
                        "Received datagram with incorrect length. While waiting for ACC", __LINE__);
    }

    if (packet_id == ACC_ID) {
        ACC_Packet acc_packet = unpack_ACC(buffer + PACKET_ID_SIZE);
        // Otrzymany wcześniej ACC
        if (acc_packet.packet_number < expected_packet_number && acc_packet.session_id == session_id) {
            return GOT_PACKET_RECEIVED_EARLIER;
        }
        if (!is_ACC_correct(acc_packet, session_id, expected_packet_number)) {
            exit_with_error(socket_fd, buffer, line, "Received incorrect ACC", __LINE__);
        }
    } else if (packet_id == RJT_ID) {
        RJT_Packet rjt_packet = unpack_RJT(buffer + PACKET_ID_SIZE);
        if (!is_RJT_correct(rjt_packet, session_id)) {
            exit_with_error(socket_fd, buffer, line, "Received incorrect RJT", __LINE__);
        }
        exit_with_error(socket_fd, buffer, line, "Received RJT", __LINE__);
    } else if (packet_id == CONACC_ID) { // Otrzymany wcześniej CONACC
        return GOT_PACKET_RECEIVED_EARLIER;
    } else {
        exit_with_error(socket_fd, buffer, line, "Got unexpected packet. Expected ACC/RJT", __LINE__);
    }

    return SUCCESS;
}

/*
 * Dzieli dane 'data_to_send' na pakiety DATA i wysyła je za pomocą protokołu udp.
 * W przypadku błędu kończy program z niezerowym kodem wyjścia.
 */
void send_DATA_packets_udp(int32_t socket_fd, uint64_t session_id, int64_t remaining_to_send, char* data_to_send,
                           uint8_t* buffer, struct sockaddr_in server_address, bool udpr) {
    uint32_t already_sent = 0;
    uint64_t packet_number = 0;
    uint32_t attempt = 1;
    bool skip_send = false;

    while (remaining_to_send > 0) {
        uint32_t to_send = min(remaining_to_send, MAX_SIZE);

        // Wysłanie pojedyńczego pakietu danych
        if (!skip_send) {
            exit_on_error(send_DATA_udp(socket_fd, session_id, packet_number, to_send, data_to_send + already_sent, server_address),
                          socket_fd, buffer, data_to_send, "Function send_DATA_udp()", __LINE__);
        }
        skip_send = false;

        if (udpr) {
            // Czekanie na ACC lub RJT
            int32_t ret = wait_for_ACC_udp(socket_fd, session_id, data_to_send, buffer,
                                           server_address, attempt, packet_number);

            if (ret == GOT_PACKET_RECEIVED_EARLIER) {
                skip_send = true; // Otrzymanie pakietu, który dostaliśmy wcześniej, nie powoduje retransmisji DATA
                continue;
            } else if (ret == TIMEOUT) {
                attempt++;
                continue;
            }
        }

        packet_number++;
        remaining_to_send -= to_send;
        already_sent += to_send;
        attempt = 1;
    }
}

/*
 * Wysyła pakiet CONN i oczekuje na CONACC lub CONRJT. Obsługuje retransmisje.
 * Sprawdza poprawność otrzymanego CONACC/CONRJT.
 *
 * W przypadku błędu kończy program z niezerowym kodem wyjścia.
 */
void send_CONN_and_wait_for_CONACC_udp(int32_t socket_fd, uint64_t session_id, int64_t remaining_to_send, char* line,
                                       uint8_t* buffer, struct sockaddr_in server_address, bool udpr) {
    uint8_t packet_id;
    bool received = false;

    // Jest jedna próba domyślna + 'MAX_RETRANSMITS' retransmisji
    for (int attempt = 1; attempt <= 1 + MAX_RETRANSMITS && !received; attempt++) {
        // Wysłanie CONN
        exit_on_error(send_CONN_udp(socket_fd, session_id, (udpr ? UDPR_ID : UDP_ID), remaining_to_send, server_address),
                      socket_fd, buffer, line, "While sending CONN. Function send_CONN_udp()", __LINE__);

        // Czekanie na CONACC lub CONRJT
        ssize_t datagram_length = recv_datagram(socket_fd, buffer, &server_address);
        if (datagram_length <= 0) {
            if (udpr && errno == EAGAIN && attempt < 1 + MAX_RETRANSMITS) {
                print_error("Timeout while waiting for CONACC/CONRJT. Retransmitting CONN", __LINE__);
                continue;
            } else if (udpr && errno == EAGAIN && attempt == 1 + MAX_RETRANSMITS) {
                exit_with_error(socket_fd, buffer, line, "While waiting for CONACC/CONRJT. "
                                                         "Reached limit of retransmissions", __LINE__);
            } else {
                exit_with_error(socket_fd, buffer, line, "While waiting for CONACC/CONRJT. "
                                                         "Function recv_datagram()", __LINE__);
            }
        }

        packet_id = unpack_packet_id(buffer);
        if (!is_length_correct(packet_id, datagram_length)) {
            exit_with_error(socket_fd, buffer, line,
                            "Received datagram with incorrect length. While waiting for CONACC/CONRJT", __LINE__);
        }

        if (packet_id == CONACC_ID) {
            if (!is_CONACC_correct(unpack_CONACC(buffer + PACKET_ID_SIZE), session_id)) {
                exit_with_error(socket_fd, buffer, line, "Received incorrect CONACC", __LINE__);
            }
        } else if (packet_id == CONRJT_ID) { // Jak połączenie odrzucone to kończymy
            if (!is_CONRJT_correct(unpack_CONRJT(buffer + PACKET_ID_SIZE), session_id)) {
                exit_with_error(socket_fd, buffer, line, "Received incorrect CONRJT", __LINE__);
            }
            exit_with_error(socket_fd, buffer, line, "Got CONRJT packet", __LINE__);
        } else {
            exit_with_error(socket_fd, buffer, line, "Got unexpected packet. Expected CONACC/CONRJT", __LINE__);
        }

        received = true;
    }
}

/*
 * Oczekuje na pakiet RCVD i sprawdza jego poprawność.
 */
void wait_for_RCVD_udp(int32_t socket_fd, uint64_t session_id, char* line,
                       uint8_t* buffer, struct sockaddr_in server_address, bool udpr) {
    uint8_t packet_id;
    while (true) {
        ssize_t datagram_length = recv_datagram(socket_fd, buffer, &server_address);
        exit_on_error_or_zero(datagram_length,
                              socket_fd, buffer, line, "While waiting for RCVD. Function recv_datagram()", __LINE__);

        packet_id = unpack_packet_id(buffer);
        if (!is_length_correct(packet_id, datagram_length)) {
            exit_with_error(socket_fd, buffer, line,
                            "Received datagram with incorrect length. While waiting for RCVD", __LINE__);
        }

        if (packet_id == RCVD_ID) {
            RCVD_Packet rcvd_packet = unpack_RCVD(buffer + PACKET_ID_SIZE);
            if (!is_RCVD_correct(rcvd_packet, session_id)) {
                exit_with_error(socket_fd, buffer, line, "Received incorrect RCVD", __LINE__);
            }
            return;
        } else if (!udpr && packet_id == RJT_ID) {
            RJT_Packet rjt_packet = unpack_RJT(buffer + PACKET_ID_SIZE);
            if (!is_RJT_correct(rjt_packet, session_id)) {
                exit_with_error(socket_fd, buffer, line, "Received incorrect RJT", __LINE__);
            }
            exit_with_error(socket_fd, buffer, line, "Received RJT packet", __LINE__);
        } else if ((udpr && packet_id == ACC_ID && unpack_ACC(buffer + PACKET_ID_SIZE).session_id == session_id) ||
                   (udpr && packet_id == CONACC_ID && unpack_CONACC(buffer + PACKET_ID_SIZE).session_id == session_id)) {
            continue; // Ignorujemy ACC/CONACC z poprzednich pakietów
        } else {
            exit_with_error(socket_fd, buffer, line, "Got unexpected packet. Expected RCVD", __LINE__);
        }
    }
}

/*
 * Wczytuje dane z stdin i przeprowadza komunikację z serwerem.
 */
void udp(struct sockaddr_in server_address, uint64_t session_id, bool udpr) {

    // Czytanie z stdin
    char* line = NULL;
    size_t len = 0;
    int64_t remaining_to_send = getdelim(&line, &len, EOF, stdin);

    // Z dokumentacji: "Buffer (line) should be freed even if getdelim() failed"
    exit_on_error(remaining_to_send, -1, line, NULL, "Function getdelim()", __LINE__);

    int32_t socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    exit_on_error(socket_fd, socket_fd, NULL, NULL, "Function socket()", __LINE__);

    exit_on_error(set_recv_timeout(socket_fd), socket_fd, NULL, NULL, "Function set_recv_timeout()", __LINE__);

    uint8_t* buffer = malloc(LARGEST_POSSIBLE_MESSAGE);
    exit_on_null(buffer, socket_fd, line, NULL, "Function malloc()", __LINE__);

    // Wysyłanie pakietu CONN i czekanie na CONACC/CONRJT
    send_CONN_and_wait_for_CONACC_udp(socket_fd, session_id, remaining_to_send, line, buffer, server_address, udpr);

    // Wysłanie danych
    send_DATA_packets_udp(socket_fd, session_id, remaining_to_send, line, buffer, server_address, udpr);

    // Czekanie na RCVD/RJT
    wait_for_RCVD_udp(socket_fd, session_id, line, buffer, server_address, udpr);

    close(socket_fd);
    free(buffer);
    free(line);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "ERROR: usage <protocol (tcp/udp/udpr)> <host> <port>\n");
        exit(EXIT_FAILURE);
    } else if (strcmp(argv[1], "tcp") != 0 && strcmp(argv[1], "udp") != 0 && strcmp(argv[1], "udpr") != 0) {
        fprintf(stderr, "ERROR: incorrect protocol\n");
        exit(EXIT_FAILURE);
    }

    char const* protocol = argv[1];
    char const* host = argv[2];
    uint16_t port = read_port(argv[3]);

    struct sockaddr_in server_address = get_server_address(host, port);

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    srand(ts.tv_nsec);
    uint64_t session_id = random_uint64();

    signal(SIGPIPE, SIG_IGN);

    if (strcmp(protocol, "tcp") == 0) {
        tcp(server_address, session_id);
    } else if (strcmp(protocol, "udp") == 0) {
        udp(server_address, session_id, false);
    } else if (strcmp(protocol, "udpr") == 0) {
        udp(server_address, session_id, true);
    }

    return 0;
}
