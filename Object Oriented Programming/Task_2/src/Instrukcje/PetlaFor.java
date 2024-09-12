package Instrukcje;

import Debugger.Debugger;
import Stos.Stos;
import Wyrazenia.*;
import java.util.ArrayList;
import java.util.List;

public class PetlaFor extends Instrukcja{
    private final char nazwaZmiennej;
    private final Wyrazenie koniec;

    public PetlaFor(char nazwaZmiennej, Wyrazenie koniec){
        this.nazwaZmiennej = nazwaZmiennej;
        this.koniec = koniec;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){
        obsluzDebugger(this.toString(), debugger);
        dodajZmiennaPetli(stos); // dodajemy na stos zmienną która jest iteratorem pętli

        try {
            for (int i = 0; i <= koniec.oblicz(stos) - 1; i++){
                // przy kazdym przejsciu zmieniamy wartosc iterowanej zmiennej
                stos.getStosZmiennych().ustawWartoscZmiennej(nazwaZmiennej, i);

                super.wykonaj(stos, debugger);
            }
        }
        catch (IllegalStateException | ArithmeticException e) {
            obsluzBlad(e, stos); // wypisujemy informacje o bledzie oraz wartosciowanie i konczymy program
        }

        stos.getStosZmiennych().usunZeStosu(); // zdejmujemy ze stosu zmienną która jest iteratorem pętli
    }

    // tworzy i dodaje na stos zmienną która jest iteratorem pętli
    private void dodajZmiennaPetli(Stos stos){
        Zmienna zmiennaPetla = new Zmienna(nazwaZmiennej);
        zmiennaPetla.setCzyIteratorPetli(true);

        // stos przyjmuje listy zmiennych, więc nową zmienną umieszczamy w jednoelementowej liście
        List<Zmienna> listaDlaZmiennejWPetli = new ArrayList<>();
        listaDlaZmiennejWPetli.add(zmiennaPetla);
        stos.getStosZmiennych().dodajNaStos(listaDlaZmiennejWPetli);
    }

    @Override
    public String toString(){
        return "for " + nazwaZmiennej + " " + koniec.toString();
    }
}
