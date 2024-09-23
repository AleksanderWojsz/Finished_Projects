package Instrukcje;

import Debugger.Debugger;
import Stos.Stos;
import Wyjatki.*;
import Wyrazenia.*;

import java.util.List;

public class Deklaracja extends Instrukcja{

    private final char nazwa;
    private final Wyrazenie wyrazenie;
    private Blok blok; // informacja w jakim bloku znajduje się deklaracja

    public Deklaracja(char nazwa, Wyrazenie wyrazenie) {
        this.nazwa = nazwa;
        this.wyrazenie = wyrazenie;
    }

    @Override
    public void ustawBlok(Blok blok){
        this.blok = blok;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger(this.toString(), debugger);

        try{
            if (!blok.isCzyDeklaracjeZrobione()){ // deklarację chcemy robić tylko raz

                sprawdzPoprawnoscNazwyZmiennej(nazwa);
                czyTakaZmiennaJuzJest(nazwa, blok.getListaZmiennych()); // sprawdzenie czy taka zmienna byla juz zadeklarowana w tym samym bloku

                Zmienna zmienna = new Zmienna(nazwa); // tworzymy i dodajemy do listy deklarowaną zmienną
                blok.dodajZmienna(zmienna);
            }

            // za kazdym razem resetujemy wartosc (po prostu liczymy wyrażenie i przypisujemy ponownie)
            stos.getStosZmiennych().ustawWartoscZmiennej(nazwa, wyrazenie.oblicz(stos));
        }
        catch (WyjatekNiedostepnaZmienna | WyjatekDzieleniePrzezZero | WyjatekZaDuzoDeklaracji | WyjatekNiepoprawnaNazwa e) {
            obsluzBlad(e, stos);
        }
    }

    // sprawdza czy na liście zmiennych istnieje już zmienna o podanej nazwie, jeśli tak to rzuca błąd
    private static void czyTakaZmiennaJuzJest(char nazwa, List<Zmienna> listaZmiennych){
        for (Zmienna zmienna : listaZmiennych) {
            if (zmienna.getNazwa() == nazwa) {
                throw new WyjatekZaDuzoDeklaracji("Deklaracja dwóch zmiennych o takiej samej nazwie w jednym bloku (zmienna " + nazwa + ")");
            }
        }
    }

    private static void sprawdzPoprawnoscNazwyZmiennej(char znak){
        if (znak < 'a' || znak > 'z'){
            throw new WyjatekNiepoprawnaNazwa("Niepoprawna nazwa zmiennej: " + znak);
        }
    }

    @Override
    public String toString(){
        return "var " + nazwa + " " + wyrazenie;
    }
}
