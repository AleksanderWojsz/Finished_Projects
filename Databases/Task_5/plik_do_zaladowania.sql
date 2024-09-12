-- Zeby przyspieszyc zapytanie mozna grupowac elementy zliczajac ich krotnosci
WITH 
lewa AS ( -- lewa odpowiada tym zlaczeniom: 'e.src=0 AND e.tgt=f.src AND f.tgt=g.src'
    SELECT g.tgt, SUM(krotnosc_f) AS krotnosc
    FROM (
        SELECT f.tgt, count(*) as krotnosc_f
        FROM e e
        JOIN e f ON e.tgt = f.src
        WHERE e.src = 0 
        GROUP BY f.tgt
    ) AS e_f
    JOIN e g ON e_f.tgt = g.src
    GROUP BY g.tgt
),
prawa AS ( -- prawa odpowiada: 'g.tgt=h.src AND h.tgt=i.src'
    SELECT h.src, SUM(krotnosc_i) AS krotnosc
    FROM (
        SELECT i.src, count(*) as krotnosc_i
        FROM e i
        GROUP BY i.src
    ) AS i
    JOIN e h ON h.tgt = i.src
    GROUP BY h.src
)

-- Majac obie czesci wystarczy mnozyc odpowiadajace sobie krotnosci
SELECT SUM(l.krotnosc * p.krotnosc) AS wynik
FROM lewa l
JOIN prawa p ON l.tgt = p.src;
