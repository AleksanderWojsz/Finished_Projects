Implement a set of Java classes that represent instructions in the simple Macchiato programming language. The program consists of a single block, and the solution must allow the execution of these programs, printing all variables from the main block at the end. Debugging capabilities should also be provided.

Key requirements:

1. **Program Execution:**
    - Execute programs built from Java classes representing Macchiato instructions.
    - Output variable values from the main block after the program finishes.
    - No need to read Macchiato programs from text files; programs are built directly in Java code.

2. **Debugging Mode:**
    - Two modes:
        - **Normal execution**: The program runs from start to finish unless an error occurs. If an error is encountered, a message is printed, and execution stops.
        - **Debugging execution**: The program pauses before each instruction and awaits commands from standard input.

3. **Debugger Commands:**
    - `c(ontinue)`: Continue execution until the program finishes.
    - `s(tep) <number>`: Execute the specified number of steps (counting all nested instructions).
    - `d(isplay) <number>`: Display the current variable state from a specified level of the program.
    - `e(xit)`: Stop the debugger and terminate the program.

4. **Macchiato Program Features:**
    - **Variables**: Single-letter variable names (`a` to `z`), all of type `int` (Java int). Variables are scoped within blocks.
    - **Block**: A block contains variable declarations and a sequence of instructions. Variables declared in the block are visible until its end.
    - **For Loop**: Iterates for a specified number of times, using the loop variable. The value for the loop is computed before the loop starts, and changing the variable during execution doesn’t affect the loop count.
    - **If Statements**: Conditional execution based on standard comparison operators. The `else` part is optional.
    - **Assignment**: Assigns a value to a variable, computed from an expression. Errors stop execution without changing the variable.
    - **Print**: Outputs the value of an expression.
    - **Variable Declarations**: Introduce a variable within a block and initialize it.

5. **Expressions:**
    - Support for integer literals, variable values, addition, subtraction, multiplication, division, and modulo.
    - Handle errors such as division by zero or referencing undeclared variables, causing program termination with an error message.

6. **Example Program**:
    - The solution should include the example Macchiato program in the `main` method, translated into Java classes.

The classes should be structured in packages, supporting both program execution and debugging, adhering to Java conventions for exception handling and input/output operations.