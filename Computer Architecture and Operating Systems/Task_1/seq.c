#include "seq.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/**
    Struktura reprezentujaca liste elementow
    bedacych w tej samej klasie abstrakcji.
    'element' to wskaznik na strukture bedaca w klasie abstrakcji.
    'next' to wskaznik na nastepny element listy.
*/
struct abstraction_class_elements
{
    seq_t *element;
    struct abstraction_class_elements *next;
};
typedef struct abstraction_class_elements abstraction_class_elements;

/**
    Struktura reprezentujaca klase abstrakcji.
    'abstraction_class_name' to nazwa klasy abstrakcji.
    'list_of_elements' to lista elementow ktore sa w danej klasie abstrakcji.
*/
struct abstraction_class
{
    char *abstraction_class_name;
    abstraction_class_elements *list_of_elements;
};
typedef struct abstraction_class abstraction_class;

/**
    Struktura reprezentujaca pojedynczy element
    w zbiorze ciagow (kazda cyfra z ciagu ma oddzielny wezel,
    poza korzeniem, ktory nie reprezentuje zadnej wartosci).

    - 'child[]' pod indeksem 'i' ma adres wezla reprezentujacego cyfre 'i',
    lub 'NULL' jesli taki wezel nie istnieje.
    - 'seq_abstraction_class' to wskaznik na klase abstrakcji, w ktorej jest
    dany wezel (ma wartosc 'NULL' jesli wezel nie ma klasy abstrakcji).
    - 'end_word' mowi czy wezel jest koncem jakiegos slowa (true),
    czy nie (false).
*/
struct seq
{
    // tablica ma rozmiar '3' poniewaz kazdy wezel moze miec syna
    // oznaczajacego '0', '1' lub '2'
    struct seq *child[3];

    abstraction_class *seq_abstraction_class;

    bool end_word;
};
typedef struct seq seq_t;

/**
    Ustawia domyslne wartosci wezla wskazanego przez 'p'
*/
void set_default_values(seq_t *p)
{
    p->end_word = 0;
    p->child[0] = NULL;
    p->child[1] = NULL;
    p->child[2] = NULL;

    // tylko wezly bedace koncem jakiegos slowa, beda mialy klase abstrakcji
    p->seq_abstraction_class = NULL;
}

/**
    Sprawdza czy wskaznik 'new_set' nie jest NULL'em
    Daje 'false' ma wartosc 'NULL' i 'true' w.p.p
*/
bool is_memory_allocated(seq_t *new_set)
{
    if (new_set == NULL) // pamieci nie udalo sie zaalokowac
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
    Dostaje znak i zamienia go na liczbe
*/
int char_to_int(char znak)
{
    return (int)znak - '0';
}

/**
    Daje wskaznik na rodzica ostatniego elementu ciagu 's' w drzewie 'p'.
    Zaklada ze ten element zawsze wystepuje
*/
seq_t *find_parent_of_sequence(seq_t *p, char const *s)
{
    for (unsigned long int i = 0; i < strlen(s) - 1; i++)
    {
        // sprawdzenie czy kolejne elementy ciagu 's' pokrywaja sie
        // z kolejnymi elementami ciagu w drzewie 'p'
        if (p->child[char_to_int(s[i])] != NULL)
        {
            p = p->child[char_to_int(s[i])];
        }
    }

    return p;
}

/**
    Daje wskaznik na wezel z drzewa 'p', reprezentujacy ostatni element
    podanego ciagu 's' lub 'NULL' jesli taki ciag nie istnieje.
*/
seq_t *find_last_node_of_sequence(seq_t *p, char const *s)
{
    bool found = true;
    for (unsigned long int i = 0; i < strlen(s) && found; i++)
    {
        // sprawdzenie czy kolejne elementy ciagu pokrywaja sie
        // z kolejnymi elementami ciagu w drzewie
        if (p->child[char_to_int(s[i])] != NULL)
        {
            p = p->child[char_to_int(s[i])];
        }
        else
        {
            found = false;
        }
    }

    if (found)
        return p;

    else
        return NULL;
}

/**
    Usuwa liste elementow klasy abstrakcji.

    'current_element' to poczatek listy do usuniecia
*/
void delete_list(abstraction_class_elements *current_element)
{
    while (current_element != NULL) // przechodzimy po calej liscie
    {
        // zapamietujemy adres nastepnika komorki ktora usuwamy,
        // zeby wiedziec co dalej usuwac
        abstraction_class_elements *next_element = current_element->next;
        free(current_element);
        current_element = next_element;
    }
}

/**
    Daje 'true', jesli wezel wskazany przez 'p' ma chociaz jednego syna.
    W przeciwnym przpadku daje 'false'.
*/
bool does_it_have_children(seq_t *p)
{
    return p->child[0] != NULL || p->child[1] != NULL || p->child[2] != NULL;
}

/**
    Daje 'true', jesli napis 's' zawiera tylko znaki '0', '1', '2'.
    W przeciwnym przypadku daje 'false'
*/
bool is_sequence_correct(char const *s)
{
    if (s != NULL && s[0] != '\0') // jesli napis istnieje i jest niepusty
    {
        bool result = true;

        // sprawdzamy kazdy znak w napisie
        for (unsigned long int i = 0; i < strlen(s) && result; i++)
        {
            if (s[i] != '0' && s[i] != '1' && s[i] != '2')
            {
                result = false;
            }
        }
        return result;
    }
    else
    {
        return false;
    }
}

/**
    Usuwa klase abstrakcji podanego wezla 'p'
*/
void delete_abstraction_class(seq_t *p)
{
    if (p->seq_abstraction_class != NULL) // jesli element ma klase abstrakcji
    {
        // jesli klasa abstrakcji ma nazwe to usuwamy nazwe
        if (strcmp(p->seq_abstraction_class->abstraction_class_name, "") != 0)
        {
            free(p->seq_abstraction_class->abstraction_class_name);
        }

        // musimy zapamiec adres klasy abstrakcji do usuniecia,
        // bo 'p' po nastepnym 'while' juz nie bedzie na nia wskazywal
        abstraction_class *abs_class_to_delete = p->seq_abstraction_class;
        abstraction_class_elements *list =
            p->seq_abstraction_class->list_of_elements;

        // poniewaz w usuwanej klasie abstrakcji moze byc wiele elementow,
        // to zmieniamy jej wszystkie elementy tak,
        // ze nie maja one zadnej klasy abstrakcji
        while (list != NULL)
        {
            list->element->seq_abstraction_class = NULL;
            list = list->next;
        }

        delete_list(abs_class_to_delete->list_of_elements);
        free(abs_class_to_delete);
    }
}

/**
    Usuwa wezel wskazany przez 'p'
*/
void delete_node(seq_t *p)
{
    // przed usunieciem wezla trzeba usunac jego klase abstrakcji
    delete_abstraction_class(p);
    free(p);
}

/**
    Usuwa drzewo wskazane przez 'p'.

    Daje '1' jesli jakikolwiek wezel zostal usuniety, '0' w.p.p
*/
int remove_tree_recursion(seq_t *p)
{
    if (p != NULL)
    {
        // obieg postfiksowy, najpierw usuwamy synow
        remove_tree_recursion(p->child[0]);
        remove_tree_recursion(p->child[1]);
        remove_tree_recursion(p->child[2]);

        delete_node(p);
        return 1;
    }

    return 0;
}

/**
    Cofa zmiany w drzewie, dokonane przez funkcje 'seq_add'

    Usuwa wszystkie wezly wlacznie z i ponizej syna o numerze 'child_number',
    wezla pod adresem 'first_added_node_parent'
    Dodatkowo, usuwa wezel pod adresem 'new_element'
*/
void undo_changes(seq_t* new_element,
    seq_t *first_added_node_parent, int child_number)
{
    seq_t *first_added_node = first_added_node_parent->child[child_number];

    // zeby nie usuwac dwa razy tego samego elementu
    if (first_added_node != new_element)
    {
        remove_tree_recursion(first_added_node);
    }

    free(new_element);

    first_added_node_parent->child[child_number] = NULL;
}

/**
    Tworzy i dodaje liste elementow do klasy abstrakcji 'new_class'
    wskazanego wezla 'p'. Laczy wezel i klase abstrakcji.

    Daje '1' jesli udalo sie, lub '-1' jesli nie udalo sie zaalokowac pamieci
*/
int add_abstraction_class_list(seq_t *p, abstraction_class *new_class)
{
    // tworzymy liste dla klasy abstrakcji
    abstraction_class_elements *new_list =
        malloc(sizeof(abstraction_class_elements));

    if (new_list != NULL) // jesli udalo sie zaalokowac pamiec dla listy
    {
        new_class->abstraction_class_name = "";

        // dodajemy liste i klase abstrakcji do wezla
        p->seq_abstraction_class = new_class;
        new_class->list_of_elements = new_list;

        // dodajemy 'p' do listy elementow klasy abstrakcji
        new_list->element = p;

        // na poczatku nie ma drugiego elementu w liscie
        new_list->next = NULL;

        return 1;
    }
    else
    {
        free(new_class);

        return -1;
    }
}

/**
    Tworzy i dodaje klase abstrakcji dla wskazanego wezla 'p'
    Daje '1' jesli udalo sie, lub '-1' jesli nie udalo sie zaalokowac pamieci
*/
int add_abstraction_class(seq_t *p)
{
    abstraction_class * new_class = malloc(sizeof(abstraction_class));

    if (new_class == NULL) // jesli nie udalo sie zaalokowac pamieci
    {
        // ciag nie ma klasy abstrakcji
        p->seq_abstraction_class = NULL;

        return -1;
    }
    else // udalo sie zaalokowac pamiec na klase abstrakcji
    {
        // tworzymy liste dla klasy abstrakcji
        return add_abstraction_class_list(p, new_class);
    }
}

/**
    Dodaje do wezla 'p' nowe dziecko o numerze 'child_number'.

    'first_added_node_parent' to wskaznik na wskaznik
    zapamietujacy rodzica pierwszego dodanego elementu,
    na wypadek nieudanej alokacji pamieci i koniecznosci cofniecia zmian.
    'current_element' to liczba ze zbioru {0, 1, 2}, reprezentujaca
    obecnie dodawany element ciagu.

    Daje '1' jesli udalo sie dodac,
    lub '-1' jesli nie udalo sie zaalokowac pamieci
*/
int add_node_to_the_tree(seq_t **p, seq_t **first_added_node_parent,
    int *child_number, int current_element)
{
    int result = 0;

    seq_t* new_element = malloc(sizeof(seq_t));

    if (new_element != NULL) // udalo sie zaalokowac pamiec na nowy wezel
    {
        // zapisujemy pierwszy dodany element
        if (*first_added_node_parent == NULL)
        {
            *first_added_node_parent = *p;
            *child_number = current_element;
        }

        set_default_values(new_element);

        // dodajemy klase abstrakcji do nowego elementu
        result = add_abstraction_class(new_element);

        // jesli nie udalo sie dodac klasy abstrakcji
        if (result == -1)
        {
            undo_changes(new_element, *first_added_node_parent, *child_number);
        }
        else // jesli nie ma problemow z pamiecia
        {
            // dodajemy nowy wezel do drzewa
            (*p)->child[current_element] = new_element;
        }
    }
    else // nie udalo sie zaalokowac pamieci na nowy wezel
    {
        if (*first_added_node_parent != NULL)
        {
            remove_tree_recursion((*first_added_node_parent)->
                child[*child_number]);

            (*first_added_node_parent)->child[*child_number] = NULL;
        }

        result = -1;
    }

    return result;
}

/**
    Tworzy nowy pusty zbior ciagow.

    Daje wskaznik na strukture reprezentujaca zbior ciagow lub
    'NULL' - jesli wystapil blad alokowania pamieci,
    ustawia wtedy errno na ENOMEM
*/
seq_t *seq_new(void)
{
    seq_t *new_set = malloc(sizeof(seq_t));

    if (new_set == NULL) // jesli pamieci nie udalo sie zaalokowac
    {
        errno = ENOMEM;
        return NULL;
    }
    else
    {
        // ustawiamy domyslne wartosci wezla
        new_set->child[0] = NULL;
        new_set->child[1] = NULL;
        new_set->child[2] = NULL;
        new_set->seq_abstraction_class = NULL;
        new_set->end_word = false;

        return new_set;
    }
}

/**
    Rekurencyjnie usuwa caly zbior ciagow wskazany przez 'p'
*/
void seq_delete(seq_t *p)
{
    if (p != NULL)
    {
        // obieg postfiksowy, najpierw usuwamy dzieci
        seq_delete(p->child[0]);
        seq_delete(p->child[1]);
        seq_delete(p->child[2]);

        delete_node(p);
    }
}

/**
    Dodaje do zbioru ciagow podany ciąg i wszystkie niepuste podciagi
    bedace jego prefiksem.

    'p' - wskaznik na strukture reprezentujaca zbior ciagow.
    's' - wskaznik na napis reprezentujacy niepusty ciag.

    Daje '1' jesli co najmniej jeden nowy ciag zostal dodany do zbioru,
    '0' jesli zbior ciagow sie nie zmienil,
    '1' jesli ktorys z parametrow jest niepoprawny
    lub wystapil blad alokowania pamieci,
    ustawia wtedy errno odpowiednio na EINVAL lub ENOMEM.
*/
int seq_add(seq_t *p, char const *s)
{
    if (is_sequence_correct(s) && p != NULL) // jesli argumenty sa poprawne
    {
        int result = 0; // wynik dzialania funkcji

        // zmienne pamietajcace pierszy dodany wezel, jego rodzica oraz
        // ktorym dzieckiem jest, na wypadek koniecznosci cofniecia zmian
        seq_t *first_added_node_parent = NULL;
        int child_number = 0;

        // przechodzimy po ciagu 's'
        for (long unsigned int i = 0; i < strlen(s) && result != -1; i++)
        {
            // jegli nie ma elementu w drzewie, to go dodajemy
            if (p->child[char_to_int(s[i])] == NULL)
            {
                result = add_node_to_the_tree(&p, &first_added_node_parent,
                    &child_number, char_to_int(s[i]));
            }

            if (result != -1)
            {
                // idziemy do nastepnego wezla
                p = p->child[char_to_int(s[i])];

                // poniewaz dodajemy kazdy prefiks to konczymy tu slowo
                p->end_word = 1;
            }
        }

        if (result == -1)
            errno = ENOMEM;

        return result;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

/**
    Usuwa element z listy elementow klasy abstrakcji
    'list' to wskaznik na wskaznik pierwszego elementu listy
    'to_delete' to adres elementu do usuniecia
*/
void remove_from_the_list(abstraction_class_elements **list, seq_t *to_delete)
{
    abstraction_class_elements *current = *list;
    abstraction_class_elements *poprz = NULL;

    // jesli nie ma szukanego elementu i lista sie nie skonczyla
    while (current != NULL && current->element != to_delete)
    {
        poprz = current;
        current = current->next;
    }

    if (current != NULL && current->element == to_delete)
    {
        if (poprz == NULL) // zmienia się poczatek listy
        {
            *list = current->next;
        }
        else
        {
            poprz->next = current->next;
        }

        free(current);
    }
}

/**
    Usuwa ze zbioru ciagow podany ciąg i wszystkie ciagi,
    ktorych jest on prefiksem.

    'p' – wskaznik na strukture reprezentujaca zbior ciagow.
    's' – wskaznik na napis reprezentujacy niepusty ciąg.

    Daje '1' jesli co najmniej jeden ciag zostal usuniety ze zbioru,
    '0' jesli zbior ciagow sie nie zmienil,
    '1' jesli ktorys z parametrow jest niepoprawny,
    ustawia wtedy errno na EINVAL.
*/
int seq_remove(seq_t *p, char const *s)
{
    if (is_sequence_correct(s) && p != NULL) // jesli argumenty sa poprawne
    {
        seq_t *last_node = find_last_node_of_sequence(p, s);
        if (last_node != NULL) // jesli ciag istnieje
        {
            int result = 0;
            seq_t *parent = find_parent_of_sequence(p, s);

            // ostatni element usuwanego ciagu
            int last_element = char_to_int(s[strlen(s) - 1]);

            // lista elemenow w klasie abstrakcji usuwanego ciagu
            abstraction_class_elements *list =
                last_node->seq_abstraction_class->list_of_elements;

            // jesli na liscie jest wiecej niz jeden element, to
            // klasa abstrakcji jest jeszcze potrzebna
            if (list->next != NULL)
            {
                // usuwamy obecny element z listy
                remove_from_the_list(&list, last_node);
                last_node->seq_abstraction_class->list_of_elements = list;

                // zeby nie usunac klasy abstrakcji, mowimy ze jej nie ma
                last_node->seq_abstraction_class = NULL;
            }

            // usuwamy ciag i wszystkie jego podciagi
            result = remove_tree_recursion(parent->child[last_element]);

            // rodzic nie ma juz jednego z dzieci bo je usunelismy
            find_parent_of_sequence(p, s)->child[last_element] = NULL;

            return result;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

/**
    Sprawdza czy podany ciag nalezy do zbioru ciagow
    'p' - wskaźnik na strukturę reprezentującą zbiór ciągów
    's' - wskaźnik na napis reprezentujący niepusty ciąg

    Przekazuje '0' jesli ciagu nie ma,
    '1' jesli ciag jest,
    '-1' jesli argumenty sa niepoprawne
*/
int seq_valid(seq_t *p, char const *s)
{
    // jesli podane argumenty sa poprawne
    if (is_sequence_correct(s) && p != NULL)
    {
        // trzeba sprawdzic czy ostatni wezel jest koncem slowa
        return find_last_node_of_sequence(p, s) != NULL &&
            find_last_node_of_sequence(p, s)->end_word == 1;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

/**
    Daje wskaznik na napis zawierający nazwę klasy abstrakcji,
    do ktorej nalezy podany ciąg.

    'p' – wskaznik na strukture reprezentujaca zbior ciagow;
    's' – wskaznik na napis reprezentujacy niepusty ciag.

    Daje wskaznik na napis zawierajacy nazwe lub daje
    'NULL' jesli ciag nie nalezy do zbioru ciagow lub
    klasa abstrakcji zawierajaca ten ciag nie ma przypisanej nazwy,
    ustawia wtedy errno na '0'.
    Daje 'NULL' jesli ktorys z parametrow jest niepoprawny,
    ustawia wtedy errno na EINVAL
*/
char const* seq_get_name(seq_t *p, char const *s)
{
    // jesli argumenty sa poprawne
    if (is_sequence_correct(s) && p != NULL)
    {
        p = find_last_node_of_sequence(p, s);

        // ciag jest w drzwie i jego klasa abstrakcji ma nazwe
        if (p != NULL && strcmp(p->seq_abstraction_class->
            abstraction_class_name, "") != 0)
        {
            return p->seq_abstraction_class->abstraction_class_name;
        }
        else
        {
            errno = 0;
            return NULL;
        }
    }
    else // jesli argumenty sa niepoprawne
    {
        errno = EINVAL;
        return NULL;
    }
}

/**
    Zmienia nazwe klasy abstrakcji wskazanego wezla
    'p' – wskaznik na wezel, ktorego nazwe klasy abstrakcji zmieniamy
    'n' – wskaznik na napis z nowa niepusta nazwa

    - Daje '1' jesli nazwa klasy abstrakcji zostala przypisana lub zmieniona,
    - '0' jesli nazwa klasy abstrakcji nie zostala zmieniona,
    - '1' jesli wystapil blad alokowania pamieci.
*/
int change_abstraction_class_name(seq_t *p, char const *n)
{
    int result = 0; // wynik dzialania funkcji

    // jesli nowa nazwa nie jest taka sama jak stara nazwa
    if (strcmp(p->seq_abstraction_class->abstraction_class_name, n)!= 0)
    {
        char* new_name = malloc(strlen(n) + 1);

        // jesli udalo sie zaalokowac pamiec na nowa nazwe
        if (new_name != NULL)
        {
            // usuwamy stara nazwa przed nadaniem nowej
            if (strcmp(p->seq_abstraction_class->
                abstraction_class_name, "") != 0)
            {
                free(p->seq_abstraction_class-> abstraction_class_name);
            }

            // ustawiamy nowa nazwe
            strcpy(new_name, n);
            p->seq_abstraction_class->
                abstraction_class_name = new_name;

            result = 1;
        }
        else // pamieci nie udalo sie zalokowac
        {
            result = -1;
        }
    }

    return result;
}

/**
    Ustawia lub zmienia nazwe klasy abstrakcji, do ktorej nalezy podany ciag.
    'p' – wskaznik na strukture reprezentujaca zbior ciagow
    's' – wskaznik na napis reprezentujacy niepusty ciag
    'n' – wskaznik na napis z nowa niepusta nazwa

    - Daje '1' jesli nazwa klasy abstrakcji zostala przypisana lub zmieniona,
    - '0' jesli ciag nie nalezy do zbioru ciagow
    lub nazwa klasy abstrakcji nie zostala zmieniona,
    - '1' jesli ktorys z parametrow jest niepoprawny lub wystapil blad
    alokowania pamieci, ustawia wtedy errno odpowiednio na EINVAL lub ENOMEM.
*/
int seq_set_name(seq_t *p, char const *s, char const *n)
{
    // jesli podane argumenty sa popoprawne
    if (is_sequence_correct(s) && p != NULL && n != NULL && n[0] != '\0')
    {
        int result = 0;

        p = find_last_node_of_sequence(p, s);

        if (p != NULL) // jesli ciag jest w zbiorze
        {
            result = change_abstraction_class_name(p, n);
        }

        if (result == -1)
            errno = ENOMEM;

        return result;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

/**
    Zmienia nazwy klas abstrakcji wezlow 's1' i 's2' z drzewa o korzeniu
    pod adresem 'p'wedlug podanych zasad:

    Jesli obie klasy abstrakcji nie maja przypisanej nazwy lub te nazwy
    sa takie same to nic sie nie zmienia.
    Jesli klasa 's1' ma nazwe, a klasa 's2' nie ma nazwy, to
    klasa 's2' dostaje nazwe klasy 's1'
    Jesli obie klasy abstrakcji maja przypisane rozne nazwy,
    to nazwa klasy abstrakcji 's2' zmienia sie na polaczenie dwoch tych nazw.
*/
int compare_and_change_abs_class_names(seq_t *p,
    char const *s1, char const *s2)
{
    int result = 1;
    seq_t *last_node_s1 = find_last_node_of_sequence(p, s1);
    seq_t *last_node_s2 = find_last_node_of_sequence(p, s2);

    char *name_s1 =
        last_node_s1->seq_abstraction_class->abstraction_class_name;
    char *name_s2 =
        last_node_s2->seq_abstraction_class->abstraction_class_name;

    // jesli nazwy sa rozne (w tym obie niepuste)
    if (strcmp(name_s1, name_s2) != 0)
    {
        // jesli nazwa 's1' jest niepusta i nazwa 's2' jest pusta
        if (strcmp(name_s1, "") != 0 && strcmp(name_s2, "") == 0)
        {
            // to klasa abstrakcji 's2' ma nazwe klasy abstrakcji 's1'
            result = change_abstraction_class_name(last_node_s2, name_s1);
        }
        // obie nazwy sa niepuste, wiec laczymy nazwy
        else if (strcmp(name_s1, "") != 0 && strcmp(name_s2, "") != 0)
        {
            char merge[strlen(s1) + strlen(s2)];
            strcpy(merge, name_s1);
            strcat(merge, name_s2);
            result = change_abstraction_class_name(last_node_s2, merge);
        }
    }

    return result;
}

/**
    Dokleja liste elementow klasy abstrakcji wezla 'node1'
    do listy klasy abstrakcji 'node2'
*/
void merge_abstraction_classes_lists(seq_t *node1, seq_t *node2)
{
    // poczatki list elementow w klasach abstrakcji
    abstraction_class_elements *list_s1 =
        node1->seq_abstraction_class->list_of_elements;
    abstraction_class_elements *list_s2 =
        node2->seq_abstraction_class->list_of_elements;

    // dodajemy liste elementow klasy abstrakcji 's1'
    // do listy elementow klasy abstrakcji 's2'
    while (list_s2->next != NULL)
    {
        list_s2 = list_s2->next;
    }
    list_s2->next = list_s1;

    // przechodzimy po elementach z pierwszej klasy 's1' i
    // zmieniamy ich klase abstrakcji na klase 's2'
    while (list_s1 != NULL)
    {
        list_s1->element->seq_abstraction_class =
            node2->seq_abstraction_class;

        list_s1 = list_s1->next;
    }
}

/**
    Laczy w jedna klase abstrakcji klasy abstrakcji
    reprezentowane przez podane ciagi.
    - Jesli obie klasy abstrakcji nie maja przypisanej nazwy,
    to nowa klasa abstrakcji tez nie ma przypisanej nazwy.
    - Jesli dokladnie jedna z klas abstrakcji ma przypisana nazwe,
    to nowa klasa abstrakcji dostaje te nazwe.
    - Jesli obie klasy abstrakcji maja przypisane rozne nazwy,
    to nazwa nowej klasy abstrakcji powstaje przez polaczenie tych nazw.
    - Jesli obie klasy abstrakcji maja przypisane taka same nazwe,
    to ta nazwa pozostaje nazwa nowej klasy abstrakcji.

    'p' - wskaznik na strukture reprezentujaca zbior ciagow
    's1' - wskaznik na napis reprezentujacy niepusty ciag
    's2' - wskaznik na napis reprezentujacy niepusty ciag

    Daje '1' jesli powstala nowa klasa abstrakcji;
    '0' jesli nie powstala nowa klasa abstrakcji, bo podane ciagi naleza juz
    do tej samej klasy abstrakcji lub ktorys z nich
    nie nalezy do zbioru ciagow,
    '-1' jesli ktorys z parametrow jest niepoprawny lub
    wystapil blad alokowania pamieci,
    ustawia wtedy errno odpowiednio na EINVAL lub ENOMEM.
*/
int seq_equiv(seq_t *p, char const *s1, char const *s2)
{
    // jesli argumenty sa poprawne
    if (is_sequence_correct(s1) && is_sequence_correct(s2) && p != NULL)
    {
        seq_t *last_node_s1 = find_last_node_of_sequence(p, s1);
        seq_t *last_node_s2 = find_last_node_of_sequence(p, s2);

        // jesli oba ciagi istnieja i maja rozne klasy abstrakcji
        if (last_node_s1 != NULL && last_node_s2 != NULL &&
            last_node_s1->seq_abstraction_class->list_of_elements !=
            last_node_s2->seq_abstraction_class->list_of_elements)
        {
            int result = 1; // wynik dzialania funkcji

            // zmieniamy nazwy klas abstrakcji
            compare_and_change_abs_class_names(p, s1, s2);

            abstraction_class* to_delete =
                last_node_s1->seq_abstraction_class;

            // doklejamy liste elementow klasy 's1' z lista klasy 's2'
            merge_abstraction_classes_lists(last_node_s1, last_node_s2);

            // usuwamy niepotrzebna klase abstrakcji
            // (nie usuwamy jej listy elementow bo ta jest caly czas potrzeba)
            // jesli klasa abstrakcji ma nazwe to usuwamy nazwe
            if (strcmp(to_delete->abstraction_class_name, "") != 0)
            {
                free(to_delete->abstraction_class_name);
            }
            free(to_delete);

            return result;
        }
        else
        {
            return 0;
        }
    }
    else // niepoprawny argument
    {
        errno = EINVAL;
        return -1;
    }
}
