package Instrukcje;
import Stos.Stos;
import Debugger.Debugger;
import Wyrazenia.Zmienna;

public class Blok extends Instrukcja {

    // Informacja, czy wszystkie zmienne w bloku zostały już zadeklarowane
    // (potrzebna żeby nie robić tej samej deklaracji wielokrotnie, jeśli blok jest w pętli).
    private boolean czyDeklaracjeZrobione;

    public Blok() {
        czyDeklaracjeZrobione = false;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        this.dodajInstrukcje(new KoniecBloku());

        obsluzDebugger("Wejście do bloku", debugger);

        // chcemy żeby blok w pierwszej kolejności brał pod uwagę "aktualne" i zadeklarowane w nim zmienne,
        // więc dodajemy jego listę zmiennych na górę stosu
        stos.getStosZmiennych().dodajNaStos(getListaZmiennych());
        stos.getStosProcedur().dodajNaStos(getListaProcedur()); // dodajemy liste procedur obecnego bloku

        // wykonujemy instrukcje które są w bloku
        for (int i = 0; i < getListaInstrukcji().size(); i++) {
            getListaInstrukcji().get(i).ustawBlok(this);
            getListaInstrukcji().get(i).wykonaj(stos, debugger);
        }

        stos.getStosProcedur().usunZeStosu();
        stos.getStosZmiennych().usunZeStosu(); // usuwamy listę zmiennych obecnego bloku
        czyDeklaracjeZrobione = true; // na końcu bloku deklaracje na pewno zostaly już zrobione
    }

    public boolean isCzyDeklaracjeZrobione() {
        return czyDeklaracjeZrobione;
    }

    public void setCzyDeklaracjeZrobione(boolean czyDeklaracjeZrobione) {
        this.czyDeklaracjeZrobione = czyDeklaracjeZrobione;
    }

    public void dodajZmienna(Zmienna zmienna){
        getListaZmiennych().add(zmienna);
    }

    public void dodajProcedure(Procedura procedura){
        getListaProcedur().add(procedura);
    }
}

