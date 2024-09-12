The task involves implementing a dynamic library in C for handling sets of sequences with an equivalence relation. Each sequence consists of non-empty strings made up of the digits 0, 1, and 2. The library provides an interface for managing these sequences, where sequences are represented as strings (e.g., the sequence {0, 1, 2} is represented as "012"). The sequences can be grouped into abstract classes, which can be assigned names.

Key functions of the library include:
1. Creating (`seq_new`) and deleting (`seq_delete`) a set of sequences.
2. Adding (`seq_add`) and removing (`seq_remove`) sequences along with their prefixes.
3. Validating if a sequence belongs to a set (`seq_valid`).
4. Assigning or retrieving names for abstract classes of sequences (`seq_set_name`, `seq_get_name`).
5. Merging two abstract classes into one (`seq_equiv`).

