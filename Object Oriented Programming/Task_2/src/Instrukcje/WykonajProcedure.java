package Instrukcje;

import Debugger.Debugger;
import Stos.Stos;
import Wyrazenia.Wyrazenie;
import Wyrazenia.Zmienna;
import java.util.ArrayList;
import java.util.List;
import Wyjatki.*;

public class WykonajProcedure extends Instrukcja{

    private final String nazwa;
    private final List<Wyrazenie> wartosciArgumentow; // Lista wyrażeń podanych jako argumenty przy wywołaniu procedury.

    public WykonajProcedure(String nazwa, List<Wyrazenie> wartosciArgumentow) {
        this.nazwa = nazwa;
        this.wartosciArgumentow = wartosciArgumentow;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger(toString(), debugger);

        try{
            // Szukamy na stosie, po nazwie, deklaracji obecnie wykonywanej procedury.
            Procedura procedura =  stos.getStosProcedur().znajdzProcedure(nazwa);

            czyDobraLiczbaArgumentow(procedura); // Sprawdzenie, czy liczba argumentów jest taka sama jak podczas deklaracji.

            // Tworzymy listę zmiennych odpowiadających argumentom i przypisujemy im wyliczone wartości.
            List<Zmienna> argumenty = new ArrayList<>();
            for (int i = 0; i < procedura.getNazwyArgumentow().size(); i++){
                Zmienna zmienna = new Zmienna(procedura.getNazwyArgumentow().get(i));
                zmienna.setWartosc(wartosciArgumentow.get(i).oblicz(stos));

                argumenty.add(zmienna);
            }

            // Dodajemy listę na górę stosu zmiennych.
            stos.getStosZmiennych().dodajNaStos(argumenty);

            // Wykonujemy znalezioną procedurę.
            for (Instrukcja instrukcja : procedura.getListaInstrukcji()) {
                instrukcja.wykonaj(stos, debugger);
            }

            stos.getStosZmiennych().usunZeStosu(); // Usuwamy ze stosu listę argumentów procedury.
        }
        catch (WyjatekNiedostepnaProcedura | WyjatekZlaLiczbaArgumentow | WyjatekNiedostepnaZmienna e){
            obsluzBlad(e, stos);
        }
    }

    // Sprawdza czy liczba argumentów podanych podczas wywołania jest poprawna,
    // tzn. czy odpowiada ona liczbie argumentów podanych przy deklaracji.
    private void czyDobraLiczbaArgumentow(Procedura procedura){
        if (procedura.getNazwyArgumentow().size() != wartosciArgumentow.size()){
            throw new WyjatekZlaLiczbaArgumentow("Zła liczba argumentów. Jest " + wartosciArgumentow.size() +
                ", powinno być " + procedura.getNazwyArgumentow().size());
        }
    }

    @Override
    public String toString(){

        // Sklejanie napisu dzielimy na dwa, ponieważ po ostatnim argumencie nie ma przecinka.
        StringBuilder argumenty = new StringBuilder();
        for (int i = 0; i < wartosciArgumentow.size() - 1; i++){
            argumenty.append(wartosciArgumentow.get(i)).append(", ");
        }

        if (wartosciArgumentow.size() > 0){
            argumenty.append(wartosciArgumentow.get(wartosciArgumentow.size() - 1));
        }

        return "Wywołanie " + nazwa + "(" + argumenty + ")";
    }
}
