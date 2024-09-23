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
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include "utilities.h"
#include "protconst.h"

#define CONTINUE 4
#define UNEXPECTED_PACKET 3
#define INCORRECT_PACKET 2
#define SUCCESS 1
#define ERROR (-1)

/*
 * Odbiera pakiety DATA od klienta i sprawdza ich poprawność.
 * Wypisuje odebrane dane na stdout.
 *
 * Jeśli otrzymany pakiet DATA jest niepoprawny, zwraca INCORRECT_PACKET.
 * Jeśli otrzymany pakiet nie jest pakietem DATA, zwraca UNEXPECTED_PACKET.
 * Jeśli pomyślnie odbierze wszystkie pakiety, zwraca SUCCESS.
 */
ssize_t receive_DATA_packets_tcp(int32_t fd, CONN_Packet conn_packet, uint64_t* incorrect_packet_number) {
    uint64_t expected_packet_number = 0;
    uint64_t remaining_to_receive = conn_packet.data_length;
    uint8_t packet_id;

    while (remaining_to_receive > 0) {
        ssize_t ret = recv_packet_id_tcp(fd, &packet_id);
        if (ret <= 0) {
            return ret;
        } else if (packet_id != DATA_ID) {
            return UNEXPECTED_PACKET;
        }

        DATA_Packet data_packet;
        ret = recv_DATA_tcp(fd, &data_packet);
        if (ret <= 0) {
            return ret;
        } else if (!is_DATA_correct(data_packet, conn_packet.session_id, expected_packet_number, remaining_to_receive)) {
            *incorrect_packet_number = data_packet.packet_number;
            return INCORRECT_PACKET;
        }

        if (printf("%.*s", (int32_t)data_packet.data_length, data_packet.data) < 0) {
            return ERROR;
        }
        fflush(stdout);

        expected_packet_number++;
        remaining_to_receive -= data_packet.data_length;
        free(data_packet.data);
    }

    return SUCCESS;
}

/*
 * Odbiera pakiet CONN i sprawdza jego poprawność.
 *
 * W przypadku błędu lub niepoprawnego pakietu CONN zwraca ERROR.
 * W przypadku sukcesu zwraca SUCCESS.
 */
int32_t wait_for_CONN_tcp(int32_t fd, CONN_Packet* conn_packet) {

    uint8_t packet_id;
    if (check_for_error_or_zero(recv_packet_id_tcp(fd, &packet_id), fd, NULL, NULL, "While waiting for CONN. "
                                                                                "Function recv()", __LINE__)) {
        return ERROR;
    }
    if (packet_id != CONN_ID) {
        print_error("Got unexpected packet while waiting for CONN", __LINE__);
        close(fd);
        return ERROR;
    }
    if (check_for_error_or_zero(recv_CONN_tcp(fd, conn_packet), fd, NULL, NULL, "While waiting for CONN. "
                                                                            "Function recv_CONN()", __LINE__)) {
        return ERROR;
    }
    if (!is_CONN_correct(*conn_packet, TCP_ID)) {
        print_error("Got incorrect CONN packet", __LINE__);
        close(fd);
        return ERROR;
    }

    return SUCCESS;
}

/*
 * W zależności od sukcesu odbioru danych (wyniku funkcji 'receive_DATA_packets_tcp()') odsyła RCVD/RJT.
 * Jeśli klient wysłał nieoczekiwany pakiet, to kończy połączenie bez odsyłania żadnej wiadomości.
 *
 * W przypadku błędu zwraca ERROR.
 * W przypadku sukcesu zwraca SUCCESS.
 *
 * 'ret' - wynik funkcji 'receive_DATA_packets_tcp()'.
 */
int32_t respond_to_received_data_tcp(int32_t fd, CONN_Packet conn_packet, ssize_t ret, uint64_t incorrect_packet_number) {
    if (ret == SUCCESS) {
        if (check_for_error_or_zero(send_RCVD_tcp(fd, conn_packet.session_id),
                                    fd, NULL, NULL, "While sending RCVD. Function send_RCVD()", __LINE__)) {
            return ERROR;
        }
    } else if (ret == INCORRECT_PACKET) {
        print_error("Got incorrect DATA packet, sending RJT", __LINE__);
        if (check_for_error_or_zero(send_RJT_tcp(fd, conn_packet.session_id, incorrect_packet_number),
                                    fd, NULL, NULL, "While sending RJT. Function send_RJT()", __LINE__)) {
            return ERROR;
        }
    } else if (ret == UNEXPECTED_PACKET) {
        print_error("Got unexpected packet while waiting for DATA", __LINE__);
        close(fd);
        return ERROR;
    }

    return SUCCESS;
}

/*
 * Obsługuje klientów tcp.
 * W przypadku błędu, po którym nie może obsłużyć żadnego klienta, kończy program z niezerowym kodem wyjścia.
 */
void tcp(uint16_t port) {
    struct sockaddr_in server_address = get_server_address("", port);

    int32_t socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    exit_on_error(socket_fd, socket_fd, NULL, NULL, "Function socket()", __LINE__);

    exit_on_error(bind(socket_fd, (struct sockaddr*)&server_address, (socklen_t)sizeof(server_address)),
                  socket_fd, NULL, NULL, "Function bind()", __LINE__);

    exit_on_error(listen(socket_fd, QUEUE_LENGTH), socket_fd, NULL, NULL, "Function listen()", __LINE__);

    while (true) {
        struct sockaddr_in client_address;
        int32_t client_fd = accept(socket_fd, (struct sockaddr*)&client_address, &((socklen_t){ sizeof(client_address) }));
        if (check_for_error(client_fd, client_fd, NULL, NULL, "Function accept()", __LINE__)) {
            continue;
        }

        if (check_for_error(set_recv_timeout(client_fd), client_fd, NULL, NULL, "Function set_recv_timeout()", __LINE__)) {
            continue;
        }

        // Odczytywanie pakietu CONN
        CONN_Packet conn_packet;
        if (wait_for_CONN_tcp(client_fd, &conn_packet) == ERROR) {
            continue;
        }

        // Odsyłanie CONACC
        if (check_for_error(send_CONACC_tcp(client_fd, conn_packet.session_id),
                            client_fd, NULL, NULL, "Function send_CONACC()", __LINE__)) {
            continue;
        }

        // Odbieranie DATA
        uint64_t incorrect_packet_number;
        ssize_t ret = receive_DATA_packets_tcp(client_fd, conn_packet, &incorrect_packet_number);
        if (check_for_error_or_zero(ret, client_fd, NULL, NULL, "Function receive_DATA_packets_tcp()", __LINE__)) {
            continue;
        }

        // Odsyłanie RCVD/RJT
        if (respond_to_received_data_tcp(client_fd, conn_packet, ret, incorrect_packet_number) == ERROR) {
            continue;
        }

        close(client_fd);
    }

    close(socket_fd);
}

/*
 * Odbiera od klienta pakiet DATA i sprawdza jego poprawność.
 *
 * W przypadku błędu zwraca ERROR.
 * W przypadku gdy należy ponowić próbę odebrania pakietu, zwraca CONTINUE.
 * W przypadku sukcesu zwraca SUCCESS.
 */
int32_t receive_single_DATA_packet_udp(int32_t socket_fd, CONN_Packet conn_packet, uint8_t* buffer,
                                       struct sockaddr_in client_address, bool* skip_client, bool udpr, uint32_t* attempt,
                                       uint64_t expected_packet_number, uint64_t remaining_to_receive, DATA_Packet* data_packet) {

    struct sockaddr_in received_address;
    ssize_t datagram_length = recv_datagram(socket_fd, buffer, &received_address);
    if (datagram_length <= 0) {
        if (udpr && errno == EAGAIN) { // Timeout; obsługiwany tylko dla udpr
            if (*attempt == 1 + MAX_RETRANSMITS) { // +1, bo domyślnie jest jedna próba i ileś retransmisji
                print_error("Reached limit of retransmissions while waiting for DATA", __LINE__);
                *skip_client = true;
                return ERROR;
            } else { // ponawiamy próbę wysłania ACC lub CONACC
                (*attempt)++;

                // Jeszcze nie dotarł żaden pakiet DATA, więc odsyłamy CONACC
                if (remaining_to_receive == conn_packet.data_length) {
                    print_error("Timeout while waiting for DATA. Retransmitting CONACC", __LINE__);
                    if (check_for_error(send_CONACC_udp(socket_fd, conn_packet.session_id, client_address),
                                        -1, NULL, NULL, "While retransmitting CONACC. Function send_CONACC_udp()", __LINE__)) {
                        *skip_client = true;
                        return ERROR;
                    }
                } else { // Odsyłanie ACC
                    print_error("Timeout while waiting for DATA. Retransmitting ACC", __LINE__);
                    if (check_for_error(send_ACC_udp(socket_fd, conn_packet.session_id, expected_packet_number - 1, client_address),
                                        -1, NULL, NULL, "While retransmitting ACC. Function send_ACC_udp()", __LINE__)) {
                        *skip_client = true;
                        return ERROR;
                    }
                }

                return CONTINUE;
            }
        } else {
            print_error("While waiting for DATA. Function recv_datagram()", __LINE__);
            *skip_client = true;
            return ERROR;
        }
    }

    uint8_t packet_id = unpack_packet_id(buffer);
    if (!is_length_correct(packet_id, datagram_length)) {
        print_error("Received datagram with incorrect length. While waiting for DATA", __LINE__);
        *skip_client = true;
        return ERROR;
    }

    if (packet_id == CONN_ID) {
        // Już jesteśmy połączeni, a inny klient próbuje się połączyć, albo obecny klient wysłał kilka razy CONN.
        // W przypadku udp zawsze to inny klient,
        // bo nie ma retransmisji i obecny klient nie mógł wysłać CONN kilkukrotnie.
        // Ignorujemy ten pakiet. Jeśli to nie aktualny klient to dodatkowo odsyłamy RJT
        if (!udpr || conn_packet.session_id != unpack_CONN(buffer + PACKET_ID_SIZE).session_id) {
            if (send_CONRJT_udp(socket_fd, unpack_CONN(buffer + PACKET_ID_SIZE).session_id, received_address) == -1) {
                print_error("While sending CONRJT. Function send_CONRJT_udp()", __LINE__);
            }
        }
        return CONTINUE;
    } else if (packet_id != DATA_ID) {
        // Aktualny klient wysłał nieoczekiwany pakiet - przestajemy go obsługiwać
        if (unpack_session_id(buffer) == conn_packet.session_id) {
            print_error("Got unexpected packet from current client. Expected DATA", __LINE__);
            *skip_client = true;
            return ERROR;
        } else { // Ktoś inny wysłał nieoczekiwany pakiet - kontynuujemy odbieranie
            print_error("Got unexpected packet from client other than current. Expected DATA", __LINE__);
            return CONTINUE;
        }
    }

    bool allocation_error = false;
    *data_packet = unpack_DATA(buffer + PACKET_ID_SIZE, &allocation_error);
    if (allocation_error) {
        print_error("Failed to allocate memory for DATA. Function malloc() in unpack_DATA()", __LINE__);
        *skip_client = true;
        return ERROR;
    }

    // Pakiet DATA, który już wcześniej dostaliśmy.
    // Zakładam, że nawet jeśli jego pole 'data_length' jest niepoprawne, to dalej obsługujemy klienta
    if (udpr && data_packet->packet_number < expected_packet_number && data_packet->session_id == conn_packet.session_id) {
        return CONTINUE; // Ignorujemy ten pakiet
    }

    if (is_DATA_correct(*data_packet, conn_packet.session_id, expected_packet_number, remaining_to_receive) == false) {
        if (data_packet->session_id == conn_packet.session_id) {
            // Aktualny klient wysłał złe dane - przestajemy go obsługiwać i odsyłamy RJT
            print_error("Got incorrect DATA packet from current client", __LINE__);
            *skip_client = true;
            free(data_packet->data);
            if (send_RJT_udp(socket_fd, data_packet->session_id, data_packet->packet_number, received_address) == -1) {
                print_error("While sending RJT packet", __LINE__);
            }
            return ERROR;
        } else { // Jak ktoś inny wysłał złe dane, to go ignorujemy i kontynuujemy odbieranie DATA
            print_error("Got incorrect DATA packet from client other than current", __LINE__);
            free(data_packet->data);
            return CONTINUE;
        }
    }

    return SUCCESS;
}

/*
 * Odbiera od klienta wszystkie pakiety DATA i wypisuje je na stdout.
 */
ssize_t receive_all_DATA_packets_udp(int32_t socket_fd, CONN_Packet conn_packet, uint8_t* buffer,
                                  struct sockaddr_in client_address, bool* skip_client, bool udpr) {

    uint64_t remaining_to_receive = conn_packet.data_length;
    uint64_t expected_packet_number = 0;
    uint32_t attempt = 1;

    while (remaining_to_receive > 0) {
        DATA_Packet data_packet;
        int32_t ret = receive_single_DATA_packet_udp(socket_fd, conn_packet, buffer, client_address, skip_client, udpr,
                                                     &attempt, expected_packet_number, remaining_to_receive, &data_packet);
        if (ret == ERROR) {
            return SUCCESS;
        } else if (ret == CONTINUE) {
            continue;
        }

        // Wysyłanie ACC
        if (udpr && check_for_error(send_ACC_udp(socket_fd, conn_packet.session_id, expected_packet_number, client_address),
                                    -1, data_packet.data, NULL, "While sending ACC. Function send_ACC_udp()", __LINE__)) {
            *skip_client = true;
            return SUCCESS;
        }

        if (printf("%.*s", (int)data_packet.data_length, data_packet.data) < 0) {
            return ERROR;
        }
        fflush(stdout);

        remaining_to_receive -= data_packet.data_length;
        expected_packet_number++;
        free(data_packet.data);
        attempt = 1;
    }

    return SUCCESS;
}

/*
 * Odbiera pakiet CONN i sprawdza jego poprawność.
 *
 * Jeśli nie udało się odebrać pakietu lub pakiet jest niepoprawny, zwraca ERROR.
 * W przeciwnym przypadku zwraca SUCCESS.
 */
int32_t wait_for_CONN_udp(int32_t socket_fd, CONN_Packet* conn_packet, uint8_t* buffer,
                      struct sockaddr_in* client_address) {

    ssize_t datagram_length = recv_datagram(socket_fd, buffer, client_address);
    if (datagram_length <= 0) {
        if (errno == EAGAIN) {
            return ERROR;
        } else {
            exit_with_error(socket_fd, buffer, NULL, "While waiting for CONN. Function recv_datagram()", __LINE__);
        }
    }
    errno = 0;

    uint8_t packet_id = unpack_packet_id(buffer);
    if (!is_length_correct(packet_id, datagram_length)) {
        print_error("Received datagram with incorrect length. While waiting for CONN", __LINE__);
        return ERROR;
    }

    if (packet_id != CONN_ID) {
        print_error("Got unexpected packet. Expected CONN", __LINE__);
        return ERROR;
    }
    *conn_packet = unpack_CONN(buffer + PACKET_ID_SIZE);
    if ((conn_packet->protocol_id != UDP_ID && conn_packet->protocol_id != UDPR_ID) || conn_packet->data_length == 0) {
        print_error("Got incorrect CONN packet", __LINE__);
        return ERROR;
    }

    return SUCCESS;
}

/*
 * Obsługuje klientów udp.
 * W przypadku błędu, po którym nie może obsłużyć żadnego klienta, kończy program z niezerowym kodem wyjścia.
 */
void udp(uint16_t port) {
    int32_t socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    exit_on_error(socket_fd, socket_fd, NULL, NULL, "Function socket()", __LINE__);

    exit_on_error(set_recv_timeout(socket_fd), socket_fd, NULL, NULL, "Function set_recv_timeout()", __LINE__);

    struct sockaddr_in server_address = get_server_address("", port);
    exit_on_error(bind(socket_fd, (struct sockaddr*)&server_address, (socklen_t)sizeof(server_address)),
                  socket_fd, NULL, NULL, "Function bind()", __LINE__);

    uint8_t* buffer = malloc(LARGEST_POSSIBLE_MESSAGE);
    exit_on_null(buffer, socket_fd, NULL, NULL, "Function malloc()", __LINE__);

    while (true) {
        struct sockaddr_in client_address;
        CONN_Packet conn_packet;

        // Odbieranie CONN
        if (wait_for_CONN_udp(socket_fd, &conn_packet, buffer, &client_address) == ERROR) {
            continue;
        }

        // Wysłanie CONACC
        if (check_for_error(send_CONACC_udp(socket_fd, conn_packet.session_id, client_address),
                            -1, NULL, NULL, "While sending CONACC. Function send_CONACC()", __LINE__)) {
            continue;
        }

        // Odbieranie danych
        bool skip_client = false;
        ssize_t ret = receive_all_DATA_packets_udp(socket_fd, conn_packet, buffer, client_address,
                                     &skip_client, conn_packet.protocol_id == UDPR_ID);

        if (check_for_error(ret,
                        -1, NULL, NULL, "While receiving DATA. Function receive_all_DATA_packets_udp()", __LINE__)) {
            continue;
        }

        // Wysłanie RCVD
        if (!skip_client && check_for_error(send_RCVD_udp(socket_fd, conn_packet.session_id, client_address),
                                            -1, NULL, NULL, "While sending RCVD. Function send_RCVD()", __LINE__)) {
            continue;
        }
    }

    free(buffer);
    close(socket_fd);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "ERROR: usage <protocol (tcp/udp)> <port>\n");
        exit(EXIT_FAILURE);
    } else if (strcmp(argv[1], "tcp") != 0 && strcmp(argv[1], "udp") != 0) {
        fprintf(stderr, "ERROR: incorrect protocol\n");
        exit(EXIT_FAILURE);
    }

    char const* protocol = argv[1];
    uint16_t port = read_port(argv[2]);

    signal(SIGPIPE, SIG_IGN);

    if (strcmp(protocol, "tcp") == 0) {
        tcp(port);
    } else if (strcmp(protocol, "udp") == 0) {
        udp(port);
    }

    return 0;
}