The task involves implementing version 1.1 of the *Macchiato* programming language with the following features and improvements:

### 1. **Procedures**:
The new version introduces procedures, which are similar to functions but return no value (void). Procedures can have parameters (integers), and their parameters are passed by value. Procedures are declared at the beginning of a block and are visible until the end of that block. Procedures can be called with arguments, and the language uses dynamic variable binding, where variables are resolved during runtime rather than at compile-time.

### 2. **New Debugger Command**:
A new debugger command (`dump`, represented by `m`) allows memory dumps to be written to a file. This memory dump includes visible procedure declarations (names and parameters) and current variable values.

### 3. **Convenient Program Creation in Java**:
Java developers can now create Macchiato programs more easily using a small SDK with a builder pattern. This allows for constructing programs step by step, similar to a DSL (Domain-Specific Language), making it easier to declare variables, procedures, and instructions in sequence.
