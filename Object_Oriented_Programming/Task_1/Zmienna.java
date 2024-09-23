public class Zmienna extends Wyrazenie{

    private final char nazwa;
    private int wartosc;
    private boolean czyIteratorPetli;

    // taki konstruktow wystarczy, bo wartosc zmiennej bedzie ustawiona poczas deklaracji
    public Zmienna(char nazwa) {
        this.nazwa = nazwa;
    }

    // Zmienną da się policzyć tylko jesli jest na stosie
    @Override
    public int oblicz(Stos stos) throws WyjatekNiedostepnaZmienna{
        return stos.znajdzZmienna(nazwa).wartosc;
    }

    @Override
    public String toString(){
        return Character.toString(nazwa);
    }

    public char getNazwa() {
        return nazwa;
    }

    public int getWartosc() {
        return wartosc;
    }

    public boolean getCzyIteratorPetli() {
        return czyIteratorPetli;
    }

    public void setWartosc(int wartosc) {
        this.wartosc = wartosc;
    }

    public void setCzyIteratorPetli(boolean czyIteratorPetli) {
        this.czyIteratorPetli = czyIteratorPetli;
    }
}
