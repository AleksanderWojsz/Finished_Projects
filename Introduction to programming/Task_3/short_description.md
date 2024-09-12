Write a program that simulates the Game of Life.

The program reads the initial generation from the input. It then repeatedly displays a portion of the board, and reads and executes user commands.

The commands control the progression through subsequent generations and define a portion of the board, referred to as the "window," that is shown to the user.

The program is parameterized by two positive integers:

- ROWS: the number of rows in the window
- COLUMNS: the number of columns in the window

These parameters can be set using symbolic constants defined with the compiler’s -D option. Default values in the code are:

- ROWS = 22
- COLUMNS = 80

The position of the window on the board is determined by the position of its top-left corner. If the top-left corner is at row i and column k, the window includes cells from row i to i + ROWS - 1 and from column k to k + COLUMNS - 1.

Initially, the top-left corner of the window is at row 1 and column 1.