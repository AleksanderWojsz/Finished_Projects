package Instrukcje;

import Debugger.Debugger;
import Stos.Stos;
import Wyjatki.*;
import Wyrazenia.Wyrazenie;

public class Przypisz extends Instrukcja{

    private final char nazwa;
    private final Wyrazenie wyrazenie;

    public Przypisz(char nazwa, Wyrazenie wyrazenie) {
        this.nazwa = nazwa;
        this.wyrazenie = wyrazenie;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger(this.toString(), debugger);

        try{
            // przypisujemy do znalezionej zmiennej podane wyrażenie
            stos.getStosZmiennych().ustawWartoscZmiennej(nazwa, wyrazenie.oblicz(stos));
        }
        catch (WyjatekNiedostepnaZmienna | WyjatekDzieleniePrzezZero e){
            obsluzBlad(e, stos);
        }
    }

    @Override
    public String toString(){
        return nazwa + " := " + wyrazenie.toString();
    }
}
