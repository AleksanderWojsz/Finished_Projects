You need to work with the publication data imported in Microproject 2. This time, write a query that calculates the publication score for each employee. The score for each publication is computed as follows: the publication’s points (column "Points") divided by the number of authors (column "Authors"). The publication score for an employee is the sum of the scores from their top 4 highest-value publications.

For example, if Kowalski has published:
- One paper with 200 points and 9 co-authors
- One paper with 140 points and 1 co-author
- Three solo papers with 200, 140, and 100 points

The values of these publications for Kowalski are:
- 200 / 10 = 20
- 140 / 2 = 70
- 200 / 1 = 200
- 140 / 1 = 140
- 100 / 1 = 100

Thus, his publication score would be the sum of the top 4 values: 200 + 140 + 100 + 70 = 510.

Submit a single text file containing one SELECT statement (the query).

The file must execute correctly with the command:

SQL> @filename.sql

in an Oracle database (or similarly in PostgreSQL).