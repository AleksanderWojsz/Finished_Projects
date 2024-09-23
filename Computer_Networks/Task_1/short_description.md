
**Byte Stream Transmission Protocol (PPCB)**

**Objective:**

Implement and test a protocol for transmitting byte streams between a client and a server.

**Protocol Overview:**

PPCB facilitates byte stream transmission using TCP or UDP. UDP connections can optionally support retransmissions. Implement three versions: TCP, UDP, and UDP with retransmission. Data is sent in packets ranging from 1 to 64,000 bytes.

**Key Protocol Stages:**

1. **Connection Establishment:**
    - **TCP:** Client connects to the server, which accepts or rejects the connection.
    - **UDP:** Client sends a `CONN` packet, server may accept (`CONACC`) or reject (`CONRJT`).

2. **Data Transmission:**
    - Client sends `DATA` packets until all data is transmitted.
    - Server verifies the packets. For UDP with retransmission, it sends an `ACC` acknowledgment. If invalid, it sends a `RJT` rejection.

3. **Termination:**
    - After receiving all data, the server sends a `RCVD` packet to confirm complete data reception. The connection is then closed.
    - Clients close the connection after receiving `RCVD`.

**Retransmission Mechanism:**
- Packets must be retransmitted up to a specified number of times if acknowledgments are not received within a timeout period.

**Packet Types:**
- **CONN:** Connection initiation
- **CONACC:** Connection acceptance
- **CONRJT:** Connection rejection
- **DATA:** Data packet
- **ACC:** Data acknowledgment
- **RJT:** Data rejection
- **RCVD:** End-of-transmission acknowledgment

**Testing:**
- Test performance under various network conditions such as bandwidth, latency, and packet loss.

