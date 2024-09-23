**Objective:**

Develop a server and client for the Hearts card game. The server runs the game while clients represent players.

**Game Rules:**

- Hearts is played with four players using a standard 52-card deck.
- Players are seated at positions: North (N), East (E), South (S), and West (W).
- The game consists of hands where each player receives 13 cards.
- Play proceeds with players taking turns to play a card, following suit if possible.
- The player who plays the lowest card of the suit wins the trick and leads the next one.
- The goal is to avoid accumulating points. Points are given for taking cards, with different penalties for different types of cards (e.g., Hearts, Queens, etc.).

**Game Types:**

1. **Avoid taking tricks:** 1 point per trick taken.
2. **Avoid Hearts:** 1 point per Heart taken.
3. **Avoid Queens:** 5 points per Queen taken.
4. **Avoid Jacks and Kings:** 2 points per Jack or King taken.
5. **Avoid the King of Hearts:** 18 points for taking this card.
6. **Avoid the 7th and last trick:** 10 points per such trick.
7. **Bandit:** Points for all of the above violations.

**Server Parameters:**

- `-p <port>`: Port number for the server to listen on (optional).
- `-f <file>`: File with game definition (required).
- `-t <timeout>`: Maximum wait time in seconds (optional, default 5 seconds).

**Client Parameters:**

- `-h <host>`: Server IP address or hostname (required).
- `-p <port>`: Server port number (required).
- `-4` or `-6`: Specify IP version (optional).
- `-N`, `-E`, `-S`, `-W`: Position at the table (required).
- `-a`: Indicates automatic play (optional; if not given, the client is a human player).

**Communication Protocol:**

- Uses TCP with ASCII messages ending in `\r\n`.
- Messages include:
    - `IAM<miejsce>`: Client specifies their table position.
    - `BUSY<lista>`: Server informs if a position is taken.
    - `DEAL<type><start><cards>`: Server deals cards for a new hand.
    - `TRICK<number><cards>`: Request for the client to play a card.
    - `WRONG<number>`: Indicates an invalid response from the client.
    - `TAKEN<number><cards><player>`: Announces who won the trick.
    - `SCORE<player><points>...`: Shows points for the current hand.
    - `TOTAL<player><points>...`: Shows total scores for the game.

**Functionality Requirements:**

- Validate command-line parameters.
- Provide clear error messages.
- Implement a basic heuristic strategy for the automatic player.
- Output game logs for both server and automatic clients.
- End with code 0 if the game completes successfully; otherwise, code 1.

**Game Definition File Format:**

- Contains game hands descriptions in the order of:
    - `<type><starter>\n`
    - `<cards for N>\n`
    - `<cards for E>\n`
    - `<cards for S>\n`
    - `<cards for W>\n`

**Report Format:**

- Logs all sent and received messages, including errors, with timestamps and IP/port details.

**Client Interaction:**

- Provides a text interface for user commands.
- Displays game information and prompts the user to play cards.
- Commands include `cards` to show hand and `tricks` to list won tricks.