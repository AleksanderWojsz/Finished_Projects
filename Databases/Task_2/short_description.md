The attachment includes three CSV files with data about employees of the Institute of Computer Science and their publications from 2017 to 2020. The task is to:

1. Create three tables (`Author`, `Publication`, `Authorship`) with appropriate columns.
2. Insert the data from the corresponding CSV files into these tables.
3. Write a query to list pairs of employees (IDs) where, for each employee with a risk level of 1 who did not publish any work in 2020, you need to list all other employees with whom they have co-published at least one work at any time.

You need to submit a single text file containing:

- Three CREATE TABLE statements (for creating the tables)
- A series of INSERT statements (for inserting the data)
- One SELECT statement (the query)

The file must execute correctly with the command:

SQL> @filename.sql

in an Oracle database (or similarly in PostgreSQL).