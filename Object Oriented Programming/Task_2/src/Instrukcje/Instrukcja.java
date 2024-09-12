package Instrukcje;

import Wyrazenia.Zmienna;

import Stos.*;
import Debugger.*;
import java.util.ArrayList;
import java.util.List;

public abstract class Instrukcja {

    private final List<Instrukcja> listaInstrukcji;
    private final List<Zmienna> listaZmiennych;
    private final List<Procedura> listaProcedur;

    public Instrukcja() {
        listaInstrukcji = new ArrayList<>();
        listaZmiennych = new ArrayList<>();
        listaProcedur = new ArrayList<>();
    }

    protected void wykonaj(Stos stos, Debugger debugger){
        for (Instrukcja instrukcja : listaInstrukcji) {
            instrukcja.wykonaj(stos, debugger);
        }
    }

    public void dodajInstrukcje(Instrukcja instrukcja){
        listaInstrukcji.add(instrukcja);
    }

    protected void ustawBlok(Blok blok){} // klasa 'Instrukcje.Deklaracja' będzie miała nadpisaną tę metodę

    // wypisuje informacje o błędzie, obecne wartościowanie i kończy program
    protected void obsluzBlad(Exception e, Stos stos){
        System.out.println(e.getMessage() + " (Instrukcja: " + this + ")");
        System.out.println("\nWartościowanie przed wystąpieniem błędu: ");
        stos.getStosZmiennych().wypiszZmienne(0);
        System.exit(1);
    }

    // obsługuje polecenie 'step' debuggera
    protected void obsluzDebugger(String informacja, Debugger debugger){
        if (debugger.isCzyAktywny()){

            // jesli skończyliśmy wykonywać polecenie 'step' to wypisujemy następną instrukcję
            if (debugger.getLiczbaKrokowDoWykonia() == 0){
                System.out.println("Następna instrukcja: " + informacja);
            }

            // sprawdza ile kroków zostało do wykonania. Jeśli zero, to uruchamia debugger
            debugger.debuggerSprawdzStan();

            debugger.zmniejszLiczbeKrokow();
        }
    }

    public List<Instrukcja> getListaInstrukcji() {
        return listaInstrukcji;
    }

    public List<Zmienna> getListaZmiennych() {
        return listaZmiennych;
    }

    public List<Procedura> getListaProcedur() {
        return listaProcedur;
    }
}