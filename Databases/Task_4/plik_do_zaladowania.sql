WITH RECURSIVE tabela AS (

    SELECT autor, 0 as wynik
    FROM autorstwo
    WHERE autor = 'Pilipczuk Mi'
  
    UNION
  
    SELECT a1.autor, 1 as wynik
    FROM autorstwo a1
    JOIN autorstwo a2 ON a1.praca = a2.praca
    WHERE a2.autor = 'Pilipczuk Mi' AND a1.autor != 'Pilipczuk Mi'

    UNION

    SELECT a2.autor, tabela.wynik + 1
    FROM autorstwo a1
    JOIN tabela ON a1.autor = tabela.autor
    JOIN autorstwo a2 ON a1.praca = a2.praca
    WHERE a2.autor != tabela.autor AND tabela.wynik < 125
)

SELECT id, MIN(wynik) as wynik
FROM autorzy
LEFT JOIN tabela ON id = autor
GROUP BY id