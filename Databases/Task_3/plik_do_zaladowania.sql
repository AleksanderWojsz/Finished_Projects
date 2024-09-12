CREATE VIEW niepogrupowane AS 
SELECT autor, SUM(punkty / autorzy) OVER (
	PARTITION BY autor
	ORDER BY punkty / autorzy DESC
	ROWS BETWEEN 3 PRECEDING AND CURRENT ROW
) as ostatnie_cztery
FROM autorstwo
JOIN prace ON autorstwo.praca = prace.id;

SELECT autor, MAX(ostatnie_cztery) as wynik_publikacyjny
FROM niepogrupowane
GROUP BY autor;