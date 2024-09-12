The query:

```sql
SELECT count(*)  
FROM e e, e f, e g, e h, e i
WHERE e.src=0 AND e.tgt=f.src AND f.tgt=g.src AND g.tgt=h.src AND h.tgt=i.src;
```

takes several seconds to execute on table `E` with data from the `ba100k.sql` script. Your task is to rewrite this query so that it executes in under one second. Note: The rewritten query should produce the same result as the original, but it can look quite different.