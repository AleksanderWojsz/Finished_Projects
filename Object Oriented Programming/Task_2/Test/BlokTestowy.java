import Debugger.Debugger;
import Instrukcje.Blok;
import Stos.Stos;

// Blok testowy to instrukcja, która wyszukuje na stosie zmiennych
// zmienną o podanej nazwie i przypisuje jej wartość do argumentu 'wynik'.
// Dzięki temu możliwe jest podejrzenie wartości niektórych zmiennych po zakończeniu wykonania programu.
public class BlokTestowy extends Blok {

    private int wynik;
    private final char nazwa;

    protected BlokTestowy(char nazwa, int domyslnaWartosc){
        this.nazwa = nazwa;
        this.wynik = domyslnaWartosc;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){
        wynik = stos.getStosZmiennych().znajdzZmienna(nazwa).oblicz(stos);
    }

    protected int dajWynik(){
        return wynik;
    }
}