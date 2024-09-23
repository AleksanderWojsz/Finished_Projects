import java.util.ArrayList;
import java.util.List;

// Stos trzyma listy zmiennych.
// Im lista jest wyżej na stosie, tym zmienne na niej są bardziej zagnieżdżone w programie
public class Stos {

    // stos realizujemy jako listę list zmiennych
    private final List<List<Zmienna>> stosZmiennych;

    public Stos() {
        stosZmiennych = new ArrayList<>();
    }

    public void dodajNaStos(List<Zmienna> listaZmiennych) {
        stosZmiennych.add(listaZmiennych);
    }

    public void usunZeStosu() {
        stosZmiennych.remove(stosZmiennych.size() - 1);
    }

    // szuka zmiennej o podanej nazwie w listach na stosie, jeśli takiej nie ma to rzuca wyjątek
    public Zmienna znajdzZmienna(char nazwa) {
        for (int i = stosZmiennych.size() - 1; i >= 0; i--) {
            List<Zmienna> blok = stosZmiennych.get(i);
            for (Zmienna zmienna : blok) {
                if (zmienna.getNazwa() == nazwa) {
                    return zmienna;
                }
            }
        }
        throw new WyjatekNiedostepnaZmienna("Niedostepna zmienna: " + nazwa);
    }

    // wypisuje, bez powtórzeń nazw, zmienne z list na stosie zaczynając od góry stosu
    // 'ilePominac' mowi ile list patrząc od góry stosu pomijamy
    // Pomijanie bierze pod uwagę tylko listy reprezentujace zmienne w bloku
    // (tzn. bez listy w której jest iterator pętli)
    public void wypiszZmienne(int ilePominac) {
        List<Character> unikatoweNazwy = new ArrayList<>();
        int obecnaWysokosc = stosZmiennych.size() - 1;

        // pomijamy listy
        while (ilePominac > 0 && obecnaWysokosc >= 0) {
            List<Zmienna> blok = stosZmiennych.get(obecnaWysokosc);
            if (blok.size() != 1 || !blok.get(0).getCzyIteratorPetli()) {
                ilePominac--;
            }
            obecnaWysokosc--;
        }

        // Wypisywanie unikatowych nazw zmiennych na reszcie stosu
        for (int i = obecnaWysokosc; i >= 0; i--) {
            List<Zmienna> blok = stosZmiennych.get(i);
            for (Zmienna zmienna : blok) {
                if (!unikatoweNazwy.contains(zmienna.getNazwa())) {
                    System.out.println(zmienna.getNazwa() + " " + zmienna.getWartosc());
                    unikatoweNazwy.add(zmienna.getNazwa());
                }
            }
        }
    }

    // daje głębokość stosu, pomijając listy w których zmienną jest iterator pętli
    // (podaje głębokość zagnieżdżenia)
    public int glebokosc() {
        int wynik = 0;

        for (List<Zmienna> blok : stosZmiennych) {
            if (blok.size() != 1 || !blok.get(0).getCzyIteratorPetli()) { // na drugi warunek patrzymy tylko jak size = 1, wiec nie trzeba sprawdzac czy tablica jest pusta
                wynik++; // nie jest iteratorem
            }
        }

        return wynik;
    }

    public void ustawWartoscZmiennej(char nazwa, int wartosc){
        znajdzZmienna(nazwa).setWartosc(wartosc);
    }
}
