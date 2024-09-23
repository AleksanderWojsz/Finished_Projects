package Instrukcje;

import Debugger.Debugger;
import Stos.Stos;
import Wyjatki.WyjatekNiepoprawneNazwyArgumentow;
import Wyjatki.WyjatekZaDuzoDeklaracji;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Procedura extends Instrukcja{

    private final String nazwa;
    private final List<Character> nazwyArgumentow; // Lista nazw argumentów procedury.
    private Blok blok; // Informacja w jakim bloku znajduje się procedura.

    public Procedura(String nazwa, List<Character> nazwyArgumentow){
        this.nazwa = nazwa;
        this.nazwyArgumentow = nazwyArgumentow;
    }

    public String getNazwa() {
        return nazwa;
    }

    public List<Character> getNazwyArgumentow() {
        return nazwyArgumentow;
    }

    @Override
    public void ustawBlok(Blok blok){
        this.blok = blok;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger(toString(), debugger);

        // Sprawdzenie poprawności deklaracji.
        try{
            czyTakaProceduraJuzJest();
            czyPoprawneNazwyArgumentow();
            czyPoprawnaNazwaProcedury();
        }
        catch (WyjatekZaDuzoDeklaracji | WyjatekNiepoprawneNazwyArgumentow e){
            obsluzBlad(e, stos);
        }

        // Dodajemy procedurę do listy procedur bloku, w którym została zadeklarowana,
        // chyba że zostało to zrobione wcześniej.
        if(!blok.isCzyDeklaracjeZrobione()){
            blok.dodajProcedure(this);
        }
    }

    // Sprawdza czy na liście procedur jest już procedura o danej nazwie.
    private void czyTakaProceduraJuzJest(){
        for (var procedura : blok.getListaProcedur()) {
            if (procedura.getNazwa().equals(nazwa)) {
                throw new WyjatekZaDuzoDeklaracji("Deklaracja dwóch procedur o takiej samej nazwie w jednym bloku");
            }
        }
    }

    // Sprawdza, czy nie podano argumentów o takich samych nazwach i czy nazwy argumentów to litery w zakresie 'a'-'z'.
    private void czyPoprawneNazwyArgumentow(){
        Set<Character> unikalneNazwy = new HashSet<>();

        for (Character nazwa : nazwyArgumentow){
            if (!unikalneNazwy.add(nazwa)){ // Do zbioru nie da się dodać dwa razy tej samej nazwy.
                throw new WyjatekNiepoprawneNazwyArgumentow("Błąd: Powtarzające się nazwy argumentów w deklaracji procedury (argument '" + nazwa + "')");
            }
            else if (nazwa < 'a' || nazwa > 'z') // Sprawdzenie, czy nazwa zmiennej jest w zakresie 'a'-'z'.
            {
                throw new WyjatekNiepoprawneNazwyArgumentow("Błąd: Niepoprawna nazwa argumentu w deklaracji procedury (argument '" + nazwa + "')");
            }
        }
    }

    // Sprawdzenie, czy nazwa procedury składa się tylko z liter w zakresie 'a'-'z'.
    private void czyPoprawnaNazwaProcedury(){

        if (nazwa.length() == 0){
            throw new WyjatekNiepoprawneNazwyArgumentow("Błąd: Pusta nazwa procedury w deklaracji procedury");
        }

        for (int i = 0; i < nazwa.length(); i++){
            if (nazwa.charAt(i) < 'a' || nazwa.charAt(i) > 'z')
            {
                throw new WyjatekNiepoprawneNazwyArgumentow("Błąd: Niepoprawna nazwa procedury w deklaracji procedury ('" + nazwa + "')");
            }
        }
    }

    @Override
    public String toString(){

        // Sklejanie napisu dzielimy na dwa, ponieważ po ostatnim argumencie nie ma przecinka.
        StringBuilder argumenty = new StringBuilder();
        for (int i = 0; i < nazwyArgumentow.size() - 1; i++){
            argumenty.append("int ").append(nazwyArgumentow.get(i)).append(", ");
        }

        if (nazwyArgumentow.size() > 0){
            argumenty.append("int ").append(nazwyArgumentow.get(nazwyArgumentow.size() - 1));
        }

        return "Deklaracja " + nazwa + "(" + argumenty + ")";
    }
}
