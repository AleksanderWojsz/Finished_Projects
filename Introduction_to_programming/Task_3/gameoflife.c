/*************************************************************************************************

    Aleksander Wojsz 450252
    Zadanie zaliczeniowe - symulacja "Game of Life"

    Program symulujacy Gre w Zycie.

    Postac danych:
    Na wejsciu programu jest opis generacji poczatkowej i ciag polecen.
    Opis generacji wskazuje komorki, ktore sa zywe.
    Ma postac ciagu wierszy zaczynajacych sie od znaku '/'.
    W ostatnim wierszu opisu generacji jest tylko znak '/'.
    We wszystkich pozostalych wierszach po znaku '/' jest liczba calkowita,
    bedaca numerem wiersza planszy.
    Po niej jest uporzadkowany rosnaco niepusty ciag liczb calkowitych, bedacych numerami kolumn.
    Kazda z tych liczb jest poprzedzona jedna spacja.
    Wiersz postaci: '/w k1 .. kn' informuje, ze w
    wierszu planszy o numerze 'w' zyja komorki w kolumnach k1, .., kn.
    W opisie generacji ciag numerow wierszy,
    czyli liczb poprzedzonych znakami '/', jest uporzadkowany rosnaco.

    Program jest parametryzowany dwiema dodatnimi liczbami calkowitymi:
    'WIERSZE' to liczba wierszy okna, 'KOLUMNY' to liczba kolumn okna.
    Wartosci tych parametrow mozna podac podczas kompilacji.
    Domyslnie 'WIERSZE' ma wartosc 22 i 'KOLUMNY' ma wartosc 80.

    Polecenia:
    Program czyta z wejscia opis generacji poczatkowej.
    Nastepnie, w petli, pokazuje fragment planszy, po czym czyta i wykonuje polecenie uzytkownika.
    Polecenia steruja liczeniem kolejnych generacji. Okreslaja tez fragment planszy (okno),
    ktorego zawartosc jest pokazywana uzytkownikowi.
    Pozycja okna na planszy jest okreslona przez pozycje jego lewego gornego rogu.
    Jesli lewy gorny rog okna jest w wierszu w i kolumnie k, to okno obejmuje komorki, ktore
    maja numer wiersza od 'w' do 'w + WIERSZE - 1' i numer kolumny od 'k' do 'k + KOLUMNY - 1'.
    Poczatkowo lewy gorny rog okna jest w wierszu numer 1 i kolumnie numer 1.
    Po opisie generacji poczatkowej, na wejsciu programu jest ciag jednowierszowych polecen.

    Program rozpoznaje polecenia:
    ->  zakonczenia pracy.
        Ma postac wiersza ze znakiem '.' (kropka).
        Przerywa petle czytajaca polecenia i konczy prace programu.

    ->  obliczenia N-tej kolejnej generacji.
        Ma postac wiersza z dodatnia liczba calkowita N.
        Liczy N-ta kolejna generacje, zaczynajac od aktualnej.
        W szczegolnosci, polecenie 1 powoduje obliczenie nastepnej generacji.

    ->  obliczenia nastepnej generacji.
        Ma postac wiersza pustego. Jest rownowazne poleceniu 1.

    ->  zrzutu stanu aktualnej generacji.
        Ma postac wiersza z liczba 0 (zero).
        Generuje kompletny opis aktualnej generacji w takim samym formacie,
        w jakim byl wczytany przez program opis generacji poczatkowej.

    ->  przesuniecia okna.
        Ma postac wiersza z dwiema liczbami calkowitymi, 'w' oraz 'k', rozdzielonymi spacja.
        Zmienia wspolrzedne lewego gornego rogu okna, umieszczajac go w wierszu 'w' i kolumnie 'k'.

    Postac wyniku:
    Przed wczytaniem polecenia program pokazuje zawartosc okna, zaczynajac od lewego gornego rogu.
    Zawartosc okna jest opisana za pomoca 'WIERSZE' wierszy, z ktorych kazdy ma dlugosc 'KOLUMNY'.
    Po ostatnim z nich nastepuje wiersz, w ktorym jest ciag znakow '=' o dlugosci 'KOLUMNY'.
    Znak w wierszu zawartosci okna okresla stan komorki.
    Komorka zywa jest reprezentowana przez znak '0' (zero).
    Komorka martwa jest reprezentowana przez znak '.' (kropka).

*************************************************************************************************/


#include <stdio.h>
#include <stdlib.h> // pozwala na uzycie funkcji 'malloc'


/**
    Liczba wierszy wyswietlanej planszy. Domyslnie 22
*/
#ifndef WIERSZE
#define WIERSZE 22
#endif


/**
    Liczba kolumn wyswietlanej planszy. Domyslnie 80
*/
#ifndef KOLUMNY
#define KOLUMNY 80
#endif


/**
    Struktura reprezentujaca jedno pole (komorke) w grze.

    'nr_kolumny' to numer kolumny, w ktorej znajduje sie komorka.
    'nast' to wskaznik na nastepna komorke w tym samym wierszu na planszy.
    'poprz' to wskaznik na poprzednia komorke w tym samym wierszu na planszy.
    'czy_zywa' - stan komorki '0' = martwa, '1' = zywa.
    'liczba_sasiadow' informacja ilu sasiadow ma komorka.
    czy plansza zostala odpowiednio rozszerzona wzgledem tej komorki - '0' = nie, '1' = tak.
*/
typedef struct L_pola
{
    int nr_kolumny;
    struct L_pola *nast;
    struct L_pola *poprz;
    int czy_zywa;
    int liczba_sasiadow;

} L_pola;


/**
    Struktura reprezentujaca liste wierszy na planszy gry.

    Zawiera informacje o numerze wiersza planszy gry 'nr_wiersza'.
    'nast' to wskaznik na nastepna komorke, czyli na nastepny wiersz.
    'poprz' to wskaznik na poprzednia komorke, czyli na poprzedni wiersz.
    'poczatek_pola' to adres na pierwszy element listy pol (komorek),
    czyli pierwsza komorke w danym wierszu na planszy gry.
*/
typedef struct L_wierszy
{
    int nr_wiersza;
    struct L_wierszy *nast;
    struct L_wierszy *poprz;
    L_pola *poczatek_pola;

} L_wierszy;


/**
    Tworzy nowa liste reprezentujaca jeden wiersz komorek
    na planszy gry (liste kolejnych pol w jednym wierszu).
    Tworzy pierwsza komorke na tej liscie i zwraca jej adres.

    Wymaga dwoch argumentow:
    'kolumna_od_uzytkownika' to numer kolumny w ktorym bedzie dodana komorka.
    'stan_komorki' to stan komorki '0' = martwa, '1' = zywa.
*/
L_pola *stworz_liste_pola(int kolumna_od_uzytkownika, int stan_komorki)
{
    // tworzymy pierwszy element listy pol (komorek)
    L_pola *poczatek = (L_pola*)malloc(sizeof(L_pola));

    // ustawienie poczatkowych wartosci dla nowej komorki
    poczatek->nr_kolumny = kolumna_od_uzytkownika;
    poczatek->nast = NULL; // na poczatku nie ma nastepnika
    poczatek->poprz = NULL; // na poczatku nie ma poprzednika
    poczatek->czy_zywa = stan_komorki;
    poczatek->liczba_sasiadow = 0;

    return poczatek;
}


/**
    Tworzy nowa liste wierszy i nowa liste pol (komorek). Zapisuje jedna komorke do listy.
    Zwraca adres na poczatek listy wierszy.

    Wymaga dwoch argumentow:
    'kolumna_od_uzytkownika' to numer kolumny w ktorym bedzie dodana komorka.
    'wiersz_od_uzytkownika' to numer wiersza w ktorym bedzie dodana komorka.
*/
L_wierszy *stworz_liste_wierszy(int wiersz_od_uzytkownika, int kolumna_od_uzytkownika)
{
    // tworzymy pierwszy element listy wierszy
    L_wierszy *poczatek = (L_wierszy*)malloc(sizeof(L_wierszy));

    // ustawienie poczatkowych wartosci dla nowego elementu
    poczatek->nr_wiersza = wiersz_od_uzytkownika;
    poczatek->nast = NULL; // na poczatku nie ma nastepnika
    poczatek->poprz = NULL; // na poczatku nie ma poprzednika

    // 'poczatek_pola' to adres na pierwszy element listy pol
    // '1', bo pierwsza utworzona komorka zawsze jest zywa
    poczatek->poczatek_pola = stworz_liste_pola(kolumna_od_uzytkownika, 1);

    return poczatek;
}


/**
    Wstawia nowy element do listy z wierszami za podanym elementem.
    Wstawia jedna nowa komorke do nowego wiersza.

    'wsk' to wskaznik na poczatek listy wierszy, do ktorej ma byc wstawiony element.
    'wiersz_od_uzytkownika' to wiersz na planszy do ktorej ma byc dodana komorka.
    'kolumna_od_uzytkownika' to kolumna na planszy do ktorej ma byc dodana komorka.
    'stan_komorki' okresla stan komorki: '0' = martwa, '1' = zywa.
*/
void wstaw_za_dla_wierszy(L_wierszy *wsk, int wiersz_od_uzytkownika,
    int kolumna_od_uzytkownika, int stan_komorki)
{
    // tworzy nowy element listy z wierszami (nowy wiersz)
    L_wierszy *nowy_wiersz = (L_wierszy*)malloc(sizeof(L_wierszy));

    // ustawia nastepnik nowego elementu na nastepnik wskazanego elementu i na odwrot
    nowy_wiersz->poprz = wsk;
    nowy_wiersz->nast = wsk->nast;

    nowy_wiersz->nr_wiersza = wiersz_od_uzytkownika;

    // 'poczatek_pola' to adres na pierwszy element listy pol
    // Stan komorki okresla, czy chcemy wstawic zywa, czy martwa komorke
    nowy_wiersz->poczatek_pola = stworz_liste_pola(kolumna_od_uzytkownika, stan_komorki);

    // ustawia nastepnik wskazanego elementu na nowy element
    wsk->nast = nowy_wiersz;

    if (nowy_wiersz->nast != NULL) // jesli nastepnik nie jest 'NULL' to pokazuje na poprzednika
    {
        nowy_wiersz->nast->poprz = nowy_wiersz;
    }
}


/**
    Wstawia nowy element do listy z wierszami (dodaje nowy wiersz do listy)

    'lista' to wskaznik na wskaznik na poczatek listy wierszy, do ktorej ma byc wstawiony element
    'wiersz_od_uzytkownika' to wiersz na planszy do ktorego ma byc dodana komorka.
    'kolumna_od_uzytkownika' to kolumna na planszy do ktorej ma byc dodana komorka.
    'stan_komorki' to stan komorki '0' = martwa, '1' = zywa.
*/
void wstaw_wiersz_do_listy(L_wierszy **lista, int wiersz_od_uzytkownika,
    int kolumna_od_uzytkownika, int stan_komorki)
{
    L_wierszy *akt; // Zmienna pomocnicza do przechowywania aktualnego elementu listy
    L_wierszy *next; // Zmienna pomocnicza do przechowywania nastepnego elementu listy

    // jesli wstawiamy element na poczatku listy
    if ((*lista) == NULL || wiersz_od_uzytkownika <= (*lista)->nr_wiersza)
    {
        akt = *lista; // zapamietaj pierwszy element listy wierszy

        // tworzy nowy element listy wierszy
        *lista = (L_wierszy*)malloc(sizeof(L_wierszy));

        (*lista)->nast = akt; // ustaw nastepnik nowego elementu na pierwszy element listy
        (*lista)->nr_wiersza = wiersz_od_uzytkownika;
        (*lista)->poprz = NULL; // poprzednik nie istnieje

        if (akt != NULL)
        {
            akt->poprz = (*lista);
        }

        // 'poczatek_pola' to adres na pierwszy element listy pol.
        // Stan komorki okresla, czy chcemy wstawic zywa, czy martwa komorke.
        (*lista)->poczatek_pola = stworz_liste_pola(kolumna_od_uzytkownika, stan_komorki);
    }
    else // wstawiany element nie jest na poczatku listy
    {
        akt = *lista; // Przypisz do zmiennej 'akt' pierwszy element listy
        next = akt->nast; // Przypisz do zmiennej 'next' nastepnik pierwszego elementu

        // dopoki nastepnik aktualnego elementu istnieje i
        // ma mniejszy numer wiersza niz nowy element
        while (next != NULL && next->nr_wiersza < wiersz_od_uzytkownika)
        {
            akt = next; // przesuwamy wskaznik na aktualny element na kolejne pole
            next = akt->nast; // przesuwamy wskaznik na nastepny element na kolejne pole
        }

        // wstaw nowy element listy wierszy za aktualnym elementem
        wstaw_za_dla_wierszy(akt, wiersz_od_uzytkownika, kolumna_od_uzytkownika, stan_komorki);
    }
}


/**
    Wstawia nowy element (komorke) do wiersza na planszy za wybranym elementem.

    'wsk' to wskaznik na wiersz, do ktorego ma byc wstawiony element.
    'kolumna_od_uzytkownika' to kolumna na planszy do ktorej ma byc dodana komorka.
    'stan_komorki' to stan komorki '0' = martwa, '1' = zywa.
*/
void wstaw_za_dla_pola(L_pola *wsk, int kolumna_od_uzytkownika, int stan_komorki)
{
    // tworzy nowy element listy komorek
    L_pola *nowa_komorka = (L_pola*)malloc(sizeof(L_pola));

    // Ustaw nastepnik nowego elementu na nastepnik podanego elementu
    nowa_komorka->nast = wsk->nast;

    // przypisanie poczatkowych wartosci komorce
    nowa_komorka->nr_kolumny = kolumna_od_uzytkownika;
    nowa_komorka->czy_zywa = stan_komorki;
    nowa_komorka->liczba_sasiadow = 0;
    nowa_komorka->poprz = wsk; // // nastepnik pokazuje na poprzednika

    wsk->nast = nowa_komorka; // Ustaw nastepnik podanego elementu na nowy element

    if (nowa_komorka->nast != NULL) // jesli nastepnik nie jest 'NULL' to pokazuje na poprzednika
    {
        nowa_komorka->nast->poprz = nowa_komorka;
    }
}


/**
    Wstawia nowy element do wiersza na planszy.

    'lista' to wskaznik na poczatek wiersza, do ktorego ma byc wstawiona komorka.
    'wiersz_od_uzytkownika' to wiersz na planszy do ktorej ma byc dodana komorka.
    'kolumna_od_uzytkownika' to kolumna na planszy do ktorej ma byc dodana komorka.
    'stan_komorki' to stan komorki '0' = martwa, '1' = zywa.
*/
void wstaw_pole_do_wiersza(L_wierszy *lista, int wiersz_od_uzytkownika,
    int kolumna_od_uzytkownika, int stan_komorki)
{

    // dopoki nie znajdziemy szukanego wiersza
    // dane sa poprawne wiec wiersz musi byc znaleziony
    while (lista->nr_wiersza != wiersz_od_uzytkownika)
    {
        lista = lista->nast; // przejdz do nastepnego wiersza
    }

    // zmienna pomocnicza do przechowywania poczatku listy pola w aktualnym wierszu
    L_pola *poczatek = lista->poczatek_pola;

    // wskaznik na poczatek listy z komorkami
    L_pola **nowa_komorka = &poczatek;

    // zmienna pomocnicza do przechowywania aktualnego elementu listy pola
    L_pola *akt;

    // zmienna pomocnicza do przechowywania nastepnego elementu listy pola
    L_pola *next;

    // jesli wstawiamy element na poczatku listy
    if ((*nowa_komorka) == NULL || kolumna_od_uzytkownika <= (*nowa_komorka)->nr_kolumny)
    {

        akt = *nowa_komorka; // zapamietaj pierwszy element listy pol

        // tworzymy nowa komorke
        *nowa_komorka = (L_pola*)malloc(sizeof(L_pola));
        (*nowa_komorka)->nast = akt; // nastepnikiem nowego elementu jest element aktualny

        // przypisanie poczatkowych wartosci komorce
        (*nowa_komorka)->nr_kolumny = kolumna_od_uzytkownika;
        (*nowa_komorka)->czy_zywa = stan_komorki;
        (*nowa_komorka)->liczba_sasiadow = 0;
        (*nowa_komorka)->poprz = NULL; // na poczatku nie ma poprzednika

        // ustawia poczatek listy pol, w aktualnym wierszu, na nowy element
        lista->poczatek_pola = *nowa_komorka;


        if (akt != NULL) // jesli nastepnik nie jest 'NULL' to pokazuje na poprzednika
        {
            akt->poprz = (*nowa_komorka);
        }

    }
    else // jesli wstawiamy element nie na poczatku listy
    {
        akt = *nowa_komorka; // Przypisz do zmiennej 'akt' pierwszy element listy pola
        next = akt->nast; // Przypisz do zmiennej 'next' nastepnik pierwszego elementu listy pola

        // Dopoki nastepnik aktualnego elementu nie jest NULL i
        // ma mniejszy numer kolumny niz nowy element
        while (next != NULL && next->nr_kolumny < kolumna_od_uzytkownika)
        {
            akt = next; // Przypisz do zmiennej 'akt' aktualny element
            next = akt->nast; // Przypisz do zmiennej 'next' nastepnik aktualnego elementu
        }

        // wstawiamy nowy element za aktualnym polem
        wstaw_za_dla_pola(akt, kolumna_od_uzytkownika, stan_komorki);
    }
}


/**
    Sprawdza czy w liscie wierszy wystepuje element o szukanej wartosci 'nr_wiersza'
    Jesli wiersz istnieje to zwraca '1', jesli nie zwraca '0'.

    'lista' to wskaznik na poczatek listy wierszy
    'szukany_wiersz' to numer wiersza ktorego szukamy na liscie
*/
int czy_istnieje_taki_wiersz(L_wierszy *lista, int szukany_wiersz)
{
    if (lista == NULL) // jesli lista jest pusta to zaden wiersz nie istnieje
    {
        return 0;
    }
    else // jesli lista niepusta
    {
        // Dopoki nastepnik aktualnego elementu listy wierszy istnieje i
        // numer aktualnego elementu jest mniejszy niz szukany wiersz
        while (lista->nast != NULL && lista->nr_wiersza < szukany_wiersz)
        {
            lista = lista->nast; // idziemy do nastepnego elementu w liscie
        }

        // Daj '1' jesli numer aktualnego elementu jest rowny szukanemu wierszowi,
        // w przeciwnym razie '0'
        return (lista->nr_wiersza == szukany_wiersz);
    }
}


/**
    Czyta stan planszy ze standardowego wejscia i zapisuje go do listy list.
    (listy wierszy i listy komorek w kazdym wierszu)

    Zwraca wskaznik na poczatek listy wierszy.
*/
L_wierszy *wczytaj_stan_planszy()
{
    char pobrany_znak; // znak wczytany ze standardowego wejscia
    int liczba; // liczba pobrana ze standardowego wejscia
    int lista_utworzona = 0; // mowi czy lista wierszy zostala juz utworzona '0' - nie, '1' - tak
    int numer_wiersza = 0;

    // wskaznik na poczatek listy wierszy, na poczatku wskazuje na 'NULL'
    L_wierszy *poczatek_listy_wierszy = NULL;

    pobrany_znak = (char)getchar(); // pobieramy pierwszy znak w wierszu ze standardowego wejscia

    // kontynuujemy tak dlugo, jak pierwszym znakiem w wierszu jest '/'
    while (pobrany_znak == '/')
    {
        pobrany_znak = (char)getchar(); // pobranie znaku ze standardowego wejscia

        // jesli po '/' jest cyfra to kontynuujemy
        if (pobrany_znak >= '0' && pobrany_znak <= '9')
        {
            // oddajemy cyfre, ktora przed chwila pobralismy, zeby moc wczytac cala liczbe
            ungetc(pobrany_znak, stdin);

            scanf("%d", &liczba); // wczytanie liczby

            numer_wiersza = liczba; // pierwsza pobrana liczba, to numer wiersza komorki

            pobrany_znak = (char)getchar(); // pobranie kolejnego znaku ze standardowego wejscia

            // wczytujemy tak dlugo jak trafiamy na liczby albo spacje albo znak nowego wiersza
            while (pobrany_znak == '\n' || pobrany_znak == ' ' ||
                (pobrany_znak >= '0' && pobrany_znak <= '9'))
            {
                if (pobrany_znak != ' ') // jesli wczytalismy kawalek liczby to go oddajemy
                {
                    ungetc(pobrany_znak, stdin);
                }

                scanf("%d", &liczba); // pobranie liczby ze standardowego wejscia

                // jesli pobrany znak nie jest znakiem nowego wiersza,
                // to w zmiennej 'liczba' jest nowa liczba.
                if (pobrany_znak != '\n')
                {
                    // jesli lista nie jest utworzona to trzeba ja utworzyc
                    if (lista_utworzona == 0)
                    {
                        poczatek_listy_wierszy = stworz_liste_wierszy(numer_wiersza, liczba);
                        lista_utworzona = 1;
                    }
                    // lista juz jest ale trzeba utworzyc nowy wiersz w liscie
                    else if (czy_istnieje_taki_wiersz(poczatek_listy_wierszy, numer_wiersza) == 0)
                    {

                        wstaw_wiersz_do_listy(&poczatek_listy_wierszy, numer_wiersza, liczba, 1);
                    }
                    else // lista juz jest i wystarczy dodac pole do wiersza
                    {
                        wstaw_pole_do_wiersza(poczatek_listy_wierszy, numer_wiersza, liczba, 1);
                    }
                }

                pobrany_znak = (char)getchar(); // pobranie znaku ze standardowego wejscia
            }
        }
    }

    return poczatek_listy_wierszy;
}


/**
    Sprawdza czy w liscie wierszy wystepuje element o wybranej wartosci 'nr_wiersza', jesli tak,
    to sprawdza czy w odpowiadajacej liscie pol wystepuje element o wybranej wartosci 'nr_kolumny'
    Jesli tak zwraca '1', jesli nie zwraca '0'.

    'lista' to wskaznik na poczatek listy wierszy.
    'szukany_wiersz' to numer wiersza ktorego szukamy na liscie wierszy
    'szukana_kolumna' to numer kolumny ktorej szukamy na liscie pol
*/
int czy_istnieje_takie_pole(L_wierszy *lista, int szukany_wiersz, int szukana_kolumna)
{
    // tak dlugo jak lista sie nie skonczyla i mamy szanse znalezc szukany wiersz.
    while (lista != NULL && lista->nr_wiersza < szukany_wiersz)
    {
        lista = lista->nast; // idziemy dalej
    }

    // jesli lista sie nie skonczyla i znalezlismy szukany wiersz
    if (lista != NULL && lista->nr_wiersza == szukany_wiersz)
    {
        // 'poczatek_pola_pom' to poczatek wiersza z polami (komorkami)
        L_pola *poczatek_pola_pom = lista->poczatek_pola;

        // tak dlugo jak lista sie nie skonczyla i
        // mamy szanse znalezc szukana komorke w dobrej kolumnie.
        while (poczatek_pola_pom != NULL && poczatek_pola_pom->nr_kolumny < szukana_kolumna)
        {
            poczatek_pola_pom = poczatek_pola_pom->nast; // idziemy dalej
        }

        // jesli lista sie nie skonczyla i
        // znalezlismy szukana komorke zwracamy '1', jesli nie to '0'.
        return poczatek_pola_pom != NULL && poczatek_pola_pom->nr_kolumny == szukana_kolumna;
    }
    else
    {
        // jesli lista sie skonczyla albo
        // nie mamy szansy na znalezienie szukanego wiersza, to zwracamy '0'.

        return 0;
    }
}


/**
    Wypisuje na standartowe wyjscie plansze gry.
    Zawartosc okna jest opisana za pomoca WIERSZE wierszy, z ktorych kazdy ma dlugosc KOLUMNY.
    Po ostatnim z nich nastepuje wiersz, w ktorym jest ciag znakow '=' o dlugosci KOLUMNY.
    Znak w wierszu zawartosci okna okresla stan komorki.
    Komorka zywa jest reprezentowana przez znak '0' (zero),
    a komorka martwa jest reprezentowana przez znak . (kropka).

    'lista' to wskaznik na poczatek listy list reprezentujacej plansze gry,
    ktora ma zostac wypisana.

    'przesuniecie_x' to przesuniecie obrazu w osi 'x'.
    'przesuniecie_y' to przesuniecie obrazu w osi 'y'.
*/
void wypisz_plansze(L_wierszy *lista, int przesuniecie_x, int przesuniecie_y)
{
    for (int i = 1; i <= WIERSZE; i++)
    {
        // jesli istnieje szukany wiersz to sprawdzamy kazda komorke
        if (czy_istnieje_taki_wiersz(lista, i + przesuniecie_x - 1) == 1)
        {
            for (int j = 1; j <= KOLUMNY; j++)
            {
                // Dla kazdego pola w obrazie sprawdzamy czy istnieje
                // komorka o wspolrzednych 'i', 'j'. Uwzgledniamy przesuniecie obrazu,
                // dodajac je do wspolrzednych 'i', 'j' sprawdzanego pola
                if (czy_istnieje_takie_pole(lista, i + przesuniecie_x - 1,
                    j + przesuniecie_y - 1) == 1)
                {
                    // jesli taka komorka istnieje wypisujemy '0' na standardowe wyjscie
                    printf("0");
                }
                else
                {
                    printf("."); // w przeciwnym przypadku wypisujemy '.' na standardowe wyjscie
                }
            }
        }
        else // jesli nie istnieje szukany wiersz to od razu mozna wypisac puste pola
        {
            for (int j = 1; j <= KOLUMNY; j++)
            {
                printf("."); // wypisujemy '.' na standardowe wyjscie
            }
        }

        printf("\n");
    }

    for (int i = 0; i < KOLUMNY; i++) // wypisujemy wiersz znakow '=' o dlugosci 'KOLUMNY'
    {
        printf("=");
    }

    printf("\n");
}


/**
    Wypisuje na standartowe wyjscie plansze gry, w takim formacie, w jakim sa dane wejsciowe,
    czyli jako ciag wierszy zaczynajacych sie od znaku '/'.
    W ostatnim wierszu opisu generacji jest tylko znak '/'.
    We wszystkich pozostalych wierszach po znaku '/' jest liczba calkowita,
    bedaca numerem wiersza planszy.

    'lista' to wskaznik na poczatek listy list reprezentujacej plansze gry,
    ktora ma zostac wypisana.
*/
void wypisz_stan_planszy(L_wierszy *lista)
{
    // tak dlugo jak lista sie nie konczy
    while (lista != NULL)
    {
        // wypisanie numeru wiersza
        printf("/%d", lista->nr_wiersza);

        // 'lista_komorek' to poczatek listy pol (komorek) w danym wierszu
        L_pola *lista_komorek = lista->poczatek_pola;

        // Dopoki nie wychodzimy za liste komorek
        while (lista_komorek != NULL)
        {
            printf(" ");

            // Wypisz numer kolumny aktualnego elementu z listy pol
            printf("%d", lista_komorek->nr_kolumny);

            // Idziemy do nastepnej komorki
            lista_komorek = lista_komorek->nast;
        }

        printf("\n");
        lista = lista->nast; // idziemy do nastpnego wiersza
    }

    printf("/\n");
}


/**
    Sprawdza czy w wybranej liscie pol istnieja elementy o 'nr_wiersza' rownym kolejno:
    - 'poprzednie_pole'
    - 'obecne_pole',
    - 'nastepne_pole'.
    Kazdej zmiennej, jesli tak, przypisuje '1', jesli nie przypisuje '0'.
    Zwraca wskaznik na element poprzedzajacy 'poprzednie_pole'

    'lista' to wskaznik na poczatek listy z polami.
    'szukana_kolumna' to numer kolumny pod adresem 'obecne_pole'.
*/
L_pola *czy_istnieja_takie_trzy_pola(L_pola *lista, int szukana_kolumna,
    int *poprzednie_pole, int *obecne_pole, int *nastepne_pole)
{
    L_pola *poprz = NULL; // poprzednik aktualnego elementu, zeby miec poprzednik dla 'NULL'

    // sprawdzamy czy istnieje poprzednie pole wzgledem 'obecne_pole'
    // tak dlugo jak lista sie nie skonczyla i mamy szanse znalezienia dobrej kolumny
    while (lista != NULL && lista->nr_kolumny < szukana_kolumna - 1)
    {
        poprz = lista;
        lista = lista->nast; // idziemy dalej w liscie pol
    }

    // jesli lista sie nie skonczyla i znalezlismy szukana kolumne przypisujemy '1',
    // jesli nie to '0'
    *poprzednie_pole = lista != NULL &&
        lista->nr_kolumny == szukana_kolumna - 1;

    // jesli poprzednio szukana kolumna istniala, to idziemy dalej,
    // jesli nie istniala, to mozemy wlasnie na niej byc
    if (*poprzednie_pole == 1)
    {
        lista = lista->nast;
    }

    // jesli lista sie nie skonczyla i znalezlismy szukana kolumne przypisujemy '1',
    // jesli nie to '0'
    *obecne_pole = lista != NULL &&
        lista->nr_kolumny == szukana_kolumna;

    // jesli poprzednio szukana kolumna istniala, to idziemy dalej,
    // jesli nie istniala, to mozemy wlasnie na niej byc
    if (*obecne_pole == 1)
    {
        lista = lista->nast;
    }

    // jesli lista sie nie skonczyla i znalezlismy szukana kolumne przypisujemy '1',
    // jesli nie to '0'
    *nastepne_pole = lista != NULL &&
        lista->nr_kolumny == szukana_kolumna + 1;


    return poprz;
}


/**
    Rozszerza liste wierszy i liste pol, w taki sposob, ze kazda zywa komorka,
    ma martwego sasiada ('stan_komorki = 0') na wszystkich sasiadujacych i wolnych polach
    (np. jesli na planszy byla jedna zywa komorka, to doda jej 8 martwych sasiadow).

    Zwraca wskaznik do zmienionej listy wierszy.

    'lista' to wskaznik na poczatek listy wierszy.
*/
L_wierszy *rozszerz_plansze(L_wierszy *lista)
{
    L_wierszy *nowy_poczatek_listy = lista; // pomocniczy wskaznik na poczatek listy wierszy

    while (lista != NULL) // tak dlugo jak nie wychodzimy poza liste
    {

        // 'komorka' to wskaznik na poczatek listy pol (komorek) w danym wierszu.
        L_pola *komorka = lista->poczatek_pola;

        // adres komorki, od ktorej bedziemy zaczynali sprawdzanie
        // czy ma trzech gornych sasiadow z uzyciem funkcji 'czy_istnieja_takie_trzy_pola'
        L_pola *poczatek_gora = komorka;

        // adres komorki, od ktorej bedziemy zaczynali sprawdzanie
        // czy ma sasiadow po bokach z uzyciem funkcji 'czy_istnieja_takie_trzy_pola'
        L_pola *poczatek_boki = komorka;

        // adres komorki, od ktorej bedziemy zaczynali sprawdzanie
        // czy ma trzech dolnych sasiadow z uzyciem funkcji 'czy_istnieja_takie_trzy_pola'
        L_pola *poczatek_dol = komorka;

        // czy 'czy_istnieja_takie_trzy_pola' nie zostalo juz sprawdzone w
        // wierszu powyzej komorki. Nie zostalo - '1', zostalo - '0'
        int pierwsze_przejscie_gora = 1;

        // czy 'czy_istnieja_takie_trzy_pola' nie zostalo juz sprawdzone w
        // wierszu na wysokosci komorki. Nie zostalo - '1', zostalo - '0'
        int pierwsze_przejscie_boki = 1;

        // czy 'czy_istnieja_takie_trzy_pola' nie zostalo juz sprawdzone w
        // wierszu ponizej komorki. Nie zostalo - '1', zostalo - '0'
        int pierwsze_przejscie_dol = 1;

        while (komorka != NULL) // tak dlugo jak nie wychodzimy poza liste
        {
            if (komorka->czy_zywa == 1) // rozszerzamy pole tylko dla zywej komorki
            {
                // rozszerzamy  plansze powyzej komorki
                // jesli wiersz nie istnieje to go wstawiamy
                if (lista->poprz == NULL || lista->nr_wiersza - 1 != lista->poprz->nr_wiersza)
                {
                    if (lista->poprz == NULL) // jesli wstawiamy elemet na poczatku listy wierszy
                    {
                        // wstawiamy lewa gorna komorke
                        wstaw_wiersz_do_listy(&nowy_poczatek_listy,
                            lista->nr_wiersza - 1 , komorka->nr_kolumny - 1, 0);
                    }
                    // nie wstawiamy na poczatku listy wiec jej poczatek sie nie zmieni,
                    // czyli nie musimy przekazywac funkcji poczatku listy
                    else
                    {
                        // wstawiamy lewa gorna komorke
                        wstaw_wiersz_do_listy(&lista->poprz,
                            lista->nr_wiersza - 1 , komorka->nr_kolumny - 1, 0);
                    }

                    // wstawiamy dwie kolejne komorki
                    wstaw_za_dla_pola(lista->poprz->poczatek_pola, komorka->nr_kolumny, 0);
                    wstaw_za_dla_pola(lista->poprz->poczatek_pola->nast,
                        komorka->nr_kolumny + 1, 0);
                }
                else // wiersz istnieje
                {
                    int poprzednie_pole;
                    int obecne_pole;
                    int nastepne_pole;

                    // jesli poczatek_gora to 'NULL', czyli nie ma nastepnika ani poprzednika,
                    // to zaczynamy od poczatku
                    if (pierwsze_przejscie_gora == 0 && poczatek_gora == NULL)
                    {
                        poczatek_gora = lista->poprz->poczatek_pola;
                    }

                    // jesli przechodzimy po raz pierwszy, to zaczynamy od poczatku
                    if (pierwsze_przejscie_gora == 1)
                    {
                        poczatek_gora = czy_istnieja_takie_trzy_pola(lista->poprz->poczatek_pola,
                            komorka->nr_kolumny, &poprzednie_pole, &obecne_pole, &nastepne_pole);

                        pierwsze_przejscie_gora = 0;
                    }
                    else // jesli nie, to zaczynamy od miejsca gdzie ostatnio skonczylismy
                    {
                        poczatek_gora = czy_istnieja_takie_trzy_pola(poczatek_gora,
                            komorka->nr_kolumny, &poprzednie_pole, &obecne_pole, &nastepne_pole);
                    }

                    if (poprzednie_pole == 0) // jesli pole na gorze po lewej nie istnieje
                    {
                        if (poczatek_gora == NULL) // jesli wstawiamy element na poczatku
                        {
                            wstaw_pole_do_wiersza(lista->poprz,
                                lista->nr_wiersza - 1, komorka->nr_kolumny - 1, 0);
                        }
                        else // jesli element do wstawienia nie bedzie na poczatku
                        {
                            wstaw_za_dla_pola(poczatek_gora, komorka->nr_kolumny - 1, 0);
                        }
                    }
                    if (obecne_pole == 0) // jesli pole na gorze nie istnieje
                    {
                        if (poczatek_gora == NULL) // jesli wstawiamy element na poczatku
                        {
                            wstaw_pole_do_wiersza(lista->poprz,
                                lista->nr_wiersza - 1, komorka->nr_kolumny, 0);
                        }
                        else // jesli element do wstawienia nie bedzie na poczatku
                        {
                            wstaw_za_dla_pola(poczatek_gora->nast, komorka->nr_kolumny, 0);
                        }
                    }
                    if (nastepne_pole == 0) // jesli pole na gorze po prawej nie istnieje
                    {
                        if (poczatek_gora == NULL) // jesli wstawiamy element na poczatku
                        {
                            wstaw_pole_do_wiersza(lista->poprz,
                                lista->nr_wiersza - 1, komorka->nr_kolumny + 1, 0);
                        }
                        else // jesli element do wstawienia nie bedzie na poczatku
                        {
                            wstaw_za_dla_pola(poczatek_gora->nast->nast,
                                komorka->nr_kolumny + 1, 0);
                        }
                    }
                }

                // rozszerzamy na boki. Wiersz zawsze istnieje
                int poprzednie_pole;
                int obecne_pole;
                int nastepne_pole;

                // jesli poczatek_boki to 'NULL', czyli nie ma nastepnika ani poprzednika,
                // to zaczynamy od poczatku
                if (pierwsze_przejscie_boki == 0 && poczatek_boki == NULL)
                {
                    poczatek_boki = lista->poczatek_pola;
                }

                // jesli przechodzimy po raz pierwszy, to zaczynamy od poczatku
                if (pierwsze_przejscie_boki == 1)
                {
                    poczatek_boki = czy_istnieja_takie_trzy_pola(lista->poczatek_pola,
                        komorka->nr_kolumny, &poprzednie_pole, &obecne_pole, &nastepne_pole);

                    pierwsze_przejscie_boki = 0;
                }
                else // jesli nie, to zaczynamy od miejsca gdzie ostatnio skonczylismy
                {
                    poczatek_boki = czy_istnieja_takie_trzy_pola(poczatek_boki,
                        komorka->nr_kolumny, &poprzednie_pole, &obecne_pole, &nastepne_pole);
                }

                if (poprzednie_pole == 0)
                {
                    if (poczatek_boki == NULL) // jesli wstawiamy element na poczatku
                    {
                        wstaw_pole_do_wiersza(lista,
                            lista->nr_wiersza, komorka->nr_kolumny - 1, 0);
                    }
                    else // jesli element do wstawienia nie bedzie na poczatku
                    {
                        wstaw_za_dla_pola(poczatek_boki, komorka->nr_kolumny - 1, 0);
                    }
                }
                if (nastepne_pole == 0)
                {
                    // element do wstawienia nie bedzie na poczatku bo
                    // wstawiamy go po zywej komorce
                    wstaw_za_dla_pola(komorka, komorka->nr_kolumny + 1, 0);
                }

                // rozszerzamy na dole
                // jesli wiersz nie istnieje to go wstawiamy
                if (lista->nast == NULL || lista->nr_wiersza + 1 != lista->nast->nr_wiersza)
                {

                    if (lista->poprz == NULL) // jesli wstawiamy elemet na poczatku listy wierszy
                    {
                        // wstawiamy lewa dolna komorke
                        wstaw_wiersz_do_listy(&nowy_poczatek_listy,
                            lista->nr_wiersza + 1 , komorka->nr_kolumny - 1, 0);
                    }
                    // nie wstawiamy na poczatku listy wiec jej poczatek sie nie zmieni,
                    // czyli nie musimy przekazywac funkcji poczatku listy
                    else
                    {
                        // wstawiamy lewa dolna komorke
                        wstaw_wiersz_do_listy(&lista->poprz,
                            lista->nr_wiersza + 1 , komorka->nr_kolumny - 1, 0);
                    }

                    // wstawiamy dwie kolejne komorki
                    wstaw_za_dla_pola(lista->nast->poczatek_pola, komorka->nr_kolumny, 0);

                    wstaw_za_dla_pola(lista->nast->poczatek_pola->nast,
                        komorka->nr_kolumny + 1, 0);
                }
                else // wiersz istnieje
                {
                    // jesli poczatek_dol to 'NULL', czyli nie ma nastepnika ani poprzednika,
                    // to zaczynamy od poczatku
                    if (pierwsze_przejscie_dol == 0 && poczatek_dol == NULL)
                    {
                        poczatek_dol = lista->nast->poczatek_pola;
                    }

                    // jesli przechodzimy po raz pierwszy, to zaczynamy od poczatku
                    if (pierwsze_przejscie_dol == 1)
                    {
                        poczatek_dol = czy_istnieja_takie_trzy_pola(lista->nast->poczatek_pola,
                            komorka->nr_kolumny, &poprzednie_pole, &obecne_pole, &nastepne_pole);
                        pierwsze_przejscie_dol = 0;

                    }
                    else // jesli nie, to zaczynamy od miejsca gdzie ostatnio skonczylismy
                    {
                        poczatek_dol = czy_istnieja_takie_trzy_pola(poczatek_dol,
                            komorka->nr_kolumny, &poprzednie_pole, &obecne_pole, &nastepne_pole);
                    }

                    if (poprzednie_pole == 0)
                    {
                        if (poczatek_dol == NULL) // jesli wstawiamy element na poczatku
                        {
                            wstaw_pole_do_wiersza(lista->nast,
                                lista->nr_wiersza + 1, komorka->nr_kolumny - 1, 0);
                        }
                        else // jesli element do wstawienia nie bedzie na poczatku
                        {
                            wstaw_za_dla_pola(poczatek_dol, komorka->nr_kolumny - 1, 0);
                        }
                    }

                    if (obecne_pole == 0)
                    {
                        if (poczatek_dol == NULL) // jesli wstawiamy element na poczatku
                        {
                            wstaw_pole_do_wiersza(lista->nast,
                                lista->nr_wiersza + 1, komorka->nr_kolumny, 0);
                        }
                        else // jesli element do wstawienia nie bedzie na poczatku
                        {
                            wstaw_za_dla_pola(poczatek_dol->nast, komorka->nr_kolumny, 0);
                        }
                    }

                    if (nastepne_pole == 0)
                    {
                        if (poczatek_dol == NULL) // jesli wstawiamy element na poczatku
                        {
                            wstaw_pole_do_wiersza(lista->nast,
                                lista->nr_wiersza + 1, komorka->nr_kolumny + 1, 0);
                        }
                        else // jesli element do wstawienia nie bedzie na poczatku
                        {
                            wstaw_za_dla_pola(poczatek_dol->nast->nast,
                                komorka->nr_kolumny + 1, 0);
                        }
                    }
                }
            }

            komorka = komorka->nast; // idziemy do nastepnej komorki
        }

        lista = lista->nast; // idziemy do nastepnego wiersza
    }

    return nowy_poczatek_listy;
}


/**
    Zwraca adres komorki o wskazanej kolumnie z listy komorek

    'lista' to wskaznik na poczatek listy komurek
    'szukana_kolumna' to kolumna w ktorej znajduje sie szukana komorka
*/
L_pola *adres_komorki(L_pola *lista, int szukana_kolumna)
{
    // szukana kolumna zawsze musi byc, bo dodalismy odpowiednie pole wczesniej
    while (lista->nr_kolumny < szukana_kolumna)
    {
        lista = lista->nast;
    }
    // jak wychodzimy z petli  to znalezlismy szukana komorke

    return lista;
}


/**
    Dla kazdej komorki aktualizuje jej liczbe zywych sasiadow ('liczba_sasiadow').

    'lista' to wskaznik na poczatek listy wierszy.
*/
void aktualizuj_liczbe_sasiadow(L_wierszy *lista)
{
    while (lista != NULL) // tak dlugo jak lista sie nie skonczyla
    {
        // wskaznik pomocniczy na poczatek listy pol
        L_pola *komorka = lista->poczatek_pola;

        // adres komorki na planszy, ktora jest powyzej aktualnej komorki
        L_pola *gdzie_dodac_sasiada_gora;

        // adres komorki na planszy, ktora jest ponizej aktualnej komorki
        L_pola *gdzie_dodac_sasiada_dol;

        // informacja czy do 'gdzie_dodac_sasiada_gora' nie zostala jeszcze przypisana wartosc.
        // '1' - nie zostala, '0' - zostala
        int pierwsze_przejscie_gora = 1;

        // informacja czy do 'pierwsze_przejscie_dol' nie zostala jeszcze przypisana wartosc.
        // '1' - nie zostala, '0' - zostala
        int pierwsze_przejscie_dol = 1;

        while (komorka != NULL) // tak dlugo jak lista sie nie skonczyla
        {
            // jesli komorka jest zywa to zwiekszamy liczbe jej sasiadow
            if (komorka->czy_zywa == 1)
            {

                // dodanie sasiadow w wierszu o jeden mniejszym
                // jesli to pierwsze przejscie, to zaczynamy szukanie od poczatku
                if (pierwsze_przejscie_gora == 1)
                {
                    gdzie_dodac_sasiada_gora =
                        adres_komorki(lista->poprz->poczatek_pola, komorka->nr_kolumny);

                    pierwsze_przejscie_gora = 0;
                }
                // zeby nie zaczyna ponownie od poczatku,
                // zaczynamy od miejsce gdzie poprzednio skonczylismy
                else
                {
                    gdzie_dodac_sasiada_gora =
                        adres_komorki(gdzie_dodac_sasiada_gora, komorka->nr_kolumny);
                }

                // zwiekszamy liczbe sasiadow powyzej zywej komorki
                gdzie_dodac_sasiada_gora->poprz->liczba_sasiadow++;
                gdzie_dodac_sasiada_gora->liczba_sasiadow++;
                gdzie_dodac_sasiada_gora->nast->liczba_sasiadow++;


                // zwiekszamy sasiadow w tym samym wierszu
                komorka->poprz->liczba_sasiadow++;
                komorka->nast->liczba_sasiadow++;


                // dodanie sasiadow w wierszu o jeden wiekszym
                // jesli to pierwsze przejscie, to zaczynamy szukanie od poczatku
                if (pierwsze_przejscie_dol == 1)
                {
                    gdzie_dodac_sasiada_dol =
                        adres_komorki(lista->nast->poczatek_pola, komorka->nr_kolumny);

                    pierwsze_przejscie_dol = 0;
                }
                // zeby nie zaczyna ponownie od poczatku,
                // zaczynamy od miejsce gdzie poprzednio skonczylismy
                else
                {
                    gdzie_dodac_sasiada_dol =
                        adres_komorki(gdzie_dodac_sasiada_dol, komorka->nr_kolumny);
                }

                // zwiekszamy liczbe sasiadow ponizej zywej komorki
                gdzie_dodac_sasiada_dol->poprz->liczba_sasiadow++;
                gdzie_dodac_sasiada_dol->liczba_sasiadow++;
                gdzie_dodac_sasiada_dol->nast->liczba_sasiadow++;
            }

            // idziemy dalej w liscie komorek
            komorka = komorka->nast;
        }

        // idziemy dalej w liscie wierszy
        lista = lista->nast;
    }
}


/**
    Dla kazdej komorki zmienia jej stan ('czy_zywa') zgodnie z zasadami:
    W nastepnej generacji komorka bedzie zywa ('1') wtedy i tylko wtedy, gdy:
        -> w biezacej generacji jest zywa i ma dokladnie dwoch lub trzech zywych sasiadow
        -> w biezacej generacji jest martwa i ma dokladnie trzech zywych sasiadow.

    Po tej operacji, dla kazdej zywej komorki,
    wartosci 'liczba_sasiadow' ustawia na '0'.

    'lista' to wskaznik na poczatek listy wierszy.
*/
void aktualizuj_stan_komorek(L_wierszy *lista)
{
    while (lista != NULL) // tak dlugo, jak nie wychodzimy poza liste wierszy
    {
        // adres pierwszego elementu w liscie pol
        L_pola *poczatek_pola_pom = lista->poczatek_pola;

        while (poczatek_pola_pom != NULL) // tak dlugo, jak nie wychodzimy poza liste pol
        {
            // jesli jest martwa i ma dokladnie 3 zywych sasiadow to ozywa
            if (poczatek_pola_pom->czy_zywa == 0 && poczatek_pola_pom->liczba_sasiadow == 3)
            {
                // zamieniamy na nowa zywa komorke
                poczatek_pola_pom->czy_zywa = 1;
                poczatek_pola_pom->liczba_sasiadow = 0;
            }
            // jesli jest zywa i ma dokladnie 2 lub 3 zywych sasiadow to nadal jest zywa
            else if (poczatek_pola_pom->czy_zywa == 1 &&
                (poczatek_pola_pom->liczba_sasiadow == 3 ||
                poczatek_pola_pom->liczba_sasiadow == 2))
            {
                // zamieniamy na nowa komorke
                poczatek_pola_pom->liczba_sasiadow = 0;
            }
            else // w innym przypadku komorka jest niezywa
            {
                poczatek_pola_pom->czy_zywa = 0;
            }

            poczatek_pola_pom = poczatek_pola_pom->nast; // idziemy dalej w liscie komorek
        }

        lista = lista->nast; // idziemy dalej w liscie wierszy
    }
}


/**
    Usuwa caly wiersz o podanym numerze z listy wierszy.
    Moze zmienic wartosc pod adresem 'wsk_na_liste'.

    'wsk_na_liste' to wskaznik na wskaznik na poczatek listy wierszy
    'wiersz_do_usuniecia' to numer wiersza do usuniecia.
*/
void usun_wiersz(L_wierszy **wsk_na_liste, L_wierszy *wiersz_do_usuniecia)
{
    L_wierszy *lista = *wsk_na_liste; // lista to wskaznik na poczatek listy wierszy

    // jesli usuwamy pierwszy wiersz na liscie
    if (lista->nr_wiersza == wiersz_do_usuniecia->nr_wiersza)
    {
        *wsk_na_liste = lista->nast; // zmienia sie poczatek listy wierszy

        if (lista->nast != NULL) // jesli nastepnik istnieje to juz nie bedzie mial poprzednika
        {
            lista->nast->poprz = NULL;
        }

        free(lista); // usuwamy wiersz do usuniecia
    }
    else
    {
        // jesli nastepnik istnieje to zmieniamy jego poprzednika
        if (wiersz_do_usuniecia->nast != NULL)
        {
            wiersz_do_usuniecia->nast->poprz = wiersz_do_usuniecia->poprz;
        }

        // nastepnikiem elementu poprzedniego jest element nastepny
        wiersz_do_usuniecia->poprz->nast = wiersz_do_usuniecia->nast;
        free(wiersz_do_usuniecia); // usuwamy element listy
    }
}


/**
    Usuwa jedno wybrane pole (komorke) z wiersza.

    'wsk_na_liste' to wskaznik na wskaznik na poczatek listy wierszy
    'komorka_do_usuniecia' to kolumna z ktorej ma zostac usunieta komorka
    'adres_wiersza_do_usuniecia' to adres wiersza z ktorego ma byc usunieta komorka.
*/
void usun_z_wiersza(L_wierszy **wsk_na_liste,
    L_pola *komorka_do_usuniecia, L_wierszy *adres_wiersza_do_usuniecia)
{
    // jesli usuwamy pierwszy element na liscie
    if (adres_wiersza_do_usuniecia->poczatek_pola->nr_kolumny == komorka_do_usuniecia->nr_kolumny)
    {
        // wskaznik na poczatek listy komorek, z wiersza z ktorego chcemy usunac komorke
        L_pola *poczatek_wiersza = adres_wiersza_do_usuniecia->poczatek_pola;

        // poczatek listy komorek zmienia sie na nastepna komorke,
        // bo pierwsza komorka zostaje usunieta
        adres_wiersza_do_usuniecia->poczatek_pola =
            adres_wiersza_do_usuniecia->poczatek_pola->nast;

        // jesli nastepnik istnieje to juz nie bedzie mial poprzednika
        if (poczatek_wiersza->nast != NULL)
        {
            poczatek_wiersza->nast->poprz = NULL;
        }

        free(poczatek_wiersza); // usuwamy pierwsza komorke
    }
    else // jesli usuwamy element inny niz pierwszy
    {
        // nastepnikiem elementu poprzedniego jest element nastepny
        komorka_do_usuniecia->poprz->nast = komorka_do_usuniecia->nast;

        // jesli nastepnik istnieje to zmieniamy jego poprzednika
        if (komorka_do_usuniecia->nast != NULL)
        {
            komorka_do_usuniecia->nast->poprz = komorka_do_usuniecia->poprz;
        }

        free(komorka_do_usuniecia); // usuwamy komorke do usuniecia
    }

    // jesli wiersz jest pusty to go usuwamy
    if (adres_wiersza_do_usuniecia->poczatek_pola == NULL)
    {
        usun_wiersz(wsk_na_liste, adres_wiersza_do_usuniecia);
    }
}


/**
    Usuwa z listy pol martwe komorki. Po tej operacji usuwa tez puste wiersze z listy wierszy.
    Moze zmienic wartosc '*wsk_na_liste'.

    'wsk_na_lista' to wskaznik na wskaznik na poczatek listy list reprezentujacej plansze gry.
*/
void aktualizuj_liste_zywych(L_wierszy **wsk_na_lista)
{
    L_wierszy *lista = *wsk_na_lista; // wskaznik na poczatek listy wierszy
    L_wierszy *gdzie_po_usunieciu_wiersza; // wskaznik pomocniczy na nastepny wiersz
    int usunieto_wiersz; // przyjmuje (1) jesli wiersz zostal usuniety, jesli nie to (0)

    while (lista != NULL) // tak dlugo jak nie wychodzimy poza liste
    {
        // wskaznik pomocniczy na poczatek listy komorek
        L_pola *poczatek_pola_pom = lista->poczatek_pola;

        usunieto_wiersz = 0; // zakladamy ze wiersza nie usunieto

        while (poczatek_pola_pom != NULL) // tak dlugo jak nie wychodzimy poza liste
        {
            // wskaznik na nastepnik obecnej komorki
            L_pola *gdzie_po_usunieciu_pola = poczatek_pola_pom->nast;

            // jesli komorka martwa to usuwamy ja z planszy
            if (poczatek_pola_pom->czy_zywa == 0)
            {

                // jesli lista (pol)komorek jest pusta to wiersz zostal usuniety
                if (lista->poczatek_pola->nast == NULL)
                {
                    usunieto_wiersz = 1;

                    // zapamietujemy adres nastepnego wiersza
                    gdzie_po_usunieciu_wiersza = lista->nast;
                }

                // usuwamy komorke z wiersza
                usun_z_wiersza(wsk_na_lista, poczatek_pola_pom, lista);
            }

            // idziemy do zapamietanej wczesniej komorki
            poczatek_pola_pom = gdzie_po_usunieciu_pola;
        }

        // jesli usunieto wiersz to idziemy do pola 'gdzie_po_usunieciu_wiersza'
        if (usunieto_wiersz == 1)
        {
            lista = gdzie_po_usunieciu_wiersza; // idziemy do zapamietanego wczesniej wiersza
        }
        else // jesli nie usunieto obecnego wiersza to idziemy
        {
            lista = lista->nast;
        }
    }
}


/**
    Liczy nastepne generacje w symulacji.
    Moze zmienic wartosc '*wsk_na_liste'.

    'lista' to wskaznik na wskaznik na poczatek listy list reprezentujacej plansze gry.
    'liczba_generacji' to liczba generacji do policzenia.
*/
void licz_generacje(L_wierszy **lista, int liczba_generacji)
{
    for (int i = 0; i < liczba_generacji; i++) // liczymy wskazana liczbe generacji
    {
        *lista = rozszerz_plansze(*lista); // rozszerzamy plansze

        aktualizuj_liczbe_sasiadow(*lista); // dla kazdej komórki liczymy jej liczbe sasiadów

        aktualizuj_stan_komorek(*lista); // aktualizujemy czy komorka jest zywa, czy martwa

        aktualizuj_liste_zywych(lista); // aktualizujemy liste zywych komórek
    }
}


/**
    Usuwa cala liste wierszy i pol.
    Moze zmienic wartosc '*wsk_na_liste'.

    'wsk_na_lista' to wskaznik na wskaznik na poczatek listy list reprezentujacej plansze gry.
*/
void usun_cala_liste(L_wierszy **wsk_na_lista)
{
    L_wierszy *lista = *wsk_na_lista; // wskaznik na poczatek listy wierszy
    int usunieto_wiersz; // przyjmuje '1' jesli wiersz zostal usuniety, jesli nie to '0'

    while (lista != NULL) // tak dlugo jak nie wychodzimy poza liste wierszy
    {
        // wskaznik na poczatek wiersza (listy pol w wierszu)
        L_pola *poczatek_pola_pom = lista->poczatek_pola;

        usunieto_wiersz = 0; // zakladamy ze wiersza nie usunieto

        while (poczatek_pola_pom != NULL) // tak dlugo jak nie wychodzimy poza liste pol
        {
            // wskaznik na nastepnik obecnej komorki
            L_pola *gdzie_dalej = poczatek_pola_pom->nast;

            // jesli lista (pol) komorek jest pusta to wiersz zostal usuniety
            if (lista->poczatek_pola->nast == NULL)
            {
                usunieto_wiersz = 1;
            }

            // usuwamy komorke z wiersza
            usun_z_wiersza(wsk_na_lista, poczatek_pola_pom, lista);

            poczatek_pola_pom = gdzie_dalej; // idziemy do zapamietanej wczesniej komorki
        }

        if (usunieto_wiersz == 1) // jesli usunieto wiersz zaczynamy od poczatku listy wierszy
        {
            lista = *wsk_na_lista;
        }
        else // jesli nie usunieto wiersza to przechodzimy do nastepnego
        {
            lista = lista->nast;
        }
    }
}


/**
    Wywoluje kolejne funkcje, symulujac Gre w Zycie.

    Czyte ze standardowego wejscia polecenia uzytkownika.
*/
void gameoflife()
{
    int poczatek_ekranu_x = 1; // poczatkowa wspolrzedna x lewego gornego rogu obrazu
    int poczatek_ekranu_y = 1; // poczatkowa wspolrzedna y lewego gornego rogu obrazu
    int liczba; // liczba pobrana ze standardowego wejscia

    L_wierszy *poczatek_listy_wierszy = wczytaj_stan_planszy(); // wczytujemy stan planszy

    // wypisujemy plansze
    wypisz_plansze(poczatek_listy_wierszy, poczatek_ekranu_x, poczatek_ekranu_y);

    char wiersz[1000]; // miejsce na wiersz
    fgets(wiersz, sizeof(wiersz), stdin); // pobranie wiersza ze standardowego wejscia

    while (wiersz[0] != '.') // tak dlugo jak poleceniem nie jest kropka
    {
        if (wiersz[0] == '\n' || wiersz[0] == '\0') // sprawdzamy, czy wiersz jest pusty
        {
            // jesli wiersz pusty, to liczymy jedna generacje i wypisujemy plansze
            licz_generacje(&poczatek_listy_wierszy, 1);
            wypisz_plansze(poczatek_listy_wierszy, poczatek_ekranu_x, poczatek_ekranu_y);
        }
        else // jesli wiersz niepusty
        {
            int liczb_w_poleceniu = 1; // ile liczb w wierszu polecenia. Domyslnie ma wartosc '1'

            // petla konczy sie jesli skonczyl sie wiersz lub w poleceniu sa dwie liczby
            for (int i = 0; i < (int)sizeof(wiersz) && wiersz[i] != '\n' &&
                liczb_w_poleceniu == 1; i++)
            {
                // jesli w wierszu jest znak ' ', to w poleceniu sa dwie liczby
                if (wiersz[i] == ' ')
                {
                    liczb_w_poleceniu = 2;

                    // pobieramy dwie liczby ze standardowego wejscia
                    sscanf(wiersz, "%d %d", &poczatek_ekranu_x, &poczatek_ekranu_y);

                    // wypisujemy plansze
                    wypisz_plansze(poczatek_listy_wierszy, poczatek_ekranu_x, poczatek_ekranu_y);
                }
            }

            // w poleceniu jest jedna liczba,
            // czyli mamy obliczyc n-ta generacje lub wypisac plansze
            if (liczb_w_poleceniu == 1)
            {
                sscanf(wiersz, "%d", &liczba); // wczytujemy ze standardowego wejscia jedna liczbe

                if (liczba == 0) // jesli polecenie to '0'
                {
                    wypisz_stan_planszy(poczatek_listy_wierszy); // wypisujemy liste wierszy i pol

                    // wypisujemy plansze
                    wypisz_plansze(poczatek_listy_wierszy, poczatek_ekranu_x, poczatek_ekranu_y);
                }
                else // jesli polecenie to niezerowa liczba liczymy generacje i wypisujemy plansze
                {
                    licz_generacje(&poczatek_listy_wierszy, liczba);
                    wypisz_plansze(poczatek_listy_wierszy, poczatek_ekranu_x, poczatek_ekranu_y);
                }
            }
        }

        fgets(wiersz, sizeof(wiersz), stdin); // wczytujemy jeden wiersz ze standardowego wejscia
    }

    usun_cala_liste(&poczatek_listy_wierszy); // na koniec usuwamy liste wierszy i pol (komorek).
}


/**
    Wywoluje funkcje rozpoczynajaca symulacje gry w zycie.
*/
int main(void)
{
    gameoflife();

    return 0;
}
