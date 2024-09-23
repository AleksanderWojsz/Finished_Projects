package Buildery;
import Instrukcje.*;
import Wyrazenia.*;
import java.util.List;

// Wzorzec projektowy "budowniczy".
public abstract class Builder <T extends Instrukcja>{

    private T struktura; // Struktura będzie typu 'Blok' lub 'Program'.

    protected void setStruktura(T struktura){
        this.struktura = struktura;
    }

    public Builder <T> nowaZmienna(char nazwa, Wyrazenie wyrazenie){
        struktura.dodajInstrukcje(new Deklaracja(nazwa, wyrazenie)); // Dodajemy deklarację zmiennej jako instrukcję bloku/programu.
        return this;
    }

    public Builder <T> nowyBlok(Blok blok){
        struktura.dodajInstrukcje(blok); // Dodajemy deklarację nowego bloku jako instrukcję bloku/programu.
        return this;
    }

    public Builder <T> nowaProcedura(String nazwa, List<Character> nazwyArgumentow, Blok blok){
        Procedura procedura = new Procedura(nazwa, nazwyArgumentow); // Trzeba utworzyć nową procedurę.
        procedura.dodajInstrukcje(blok); // Każda procedura dostaje podany blok jako instrukcję.

        struktura.dodajInstrukcje(procedura);
        return this;
    }

    public Builder <T> wywolaj(String nazwa, List<Wyrazenie> argumenty){
        struktura.dodajInstrukcje(new WykonajProcedure(nazwa, argumenty));
        return this;
    }

    public Builder <T> petlaFor(char nazwaZmiennej, Wyrazenie koniec, Blok blok){
        PetlaFor petlaFor = new PetlaFor(nazwaZmiennej, koniec);
        petlaFor.dodajInstrukcje(blok); // Ustawienie podanego bloku jako instrukcji pętli for.

        struktura.dodajInstrukcje(petlaFor); // Dodanie bloku jako instrukcji pętli for.
        return this;
    }

    public Builder <T> warunekIf(Wyrazenie wyrazenie1, String porownanie, Wyrazenie wyrazenie2, Blok blok_if, Blok blok_else){
        WarunekIf warunekIf = new WarunekIf(wyrazenie1, porownanie, wyrazenie2);
        warunekIf.dodajInstrukcje(blok_if); // Warunek 'if' ma dwa bloki - jeden na instrukcje wykonywane jeśli warunek był spełniony i
        warunekIf.dodajInstrukcjeElse(blok_else); // drugi blok na instrukcje wykonywane w przeciwnym przypadku.

        struktura.dodajInstrukcje(warunekIf);
        return this;
    }

    public Builder <T> przypisz(char nazwa, Wyrazenie wyrazenie){
        struktura.dodajInstrukcje(new Przypisz(nazwa, wyrazenie));
        return this;
    }

    public Builder <T> wypisz(Wyrazenie wyrazenie){
        struktura.dodajInstrukcje(new Print(wyrazenie));
        return this;
    }

    public T build(){
        return struktura;
    }
}
