public class Blok extends Instrukcja{

    // informacja, czy wszystkie zmienne w bloku zostały już zadeklarowane
    // (potrzebna żeby nie robić tej samej deklaracji wielokrotnie, jeśli blok jest w pętli)
    private boolean czyDeklaracjeZrobione;

    public Blok() {
        czyDeklaracjeZrobione= false;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger("Wejście do bloku", debugger);

        // chcemy żeby blok w pierwszej kolejności brał pod uwagę "aktualne" i zadeklarowane w nim zmienne,
        // więc dodajemy jego listę zmiennych na górę stosu
        stos.dodajNaStos(getListaZmiennych());

        // wykonujemy instrukcje które są w bloku
        for (int i = 0; i < getListaInstrukcji().size(); i++) {
            getListaInstrukcji().get(i).ustawBlok(this);
            getListaInstrukcji().get(i).wykonaj(stos, debugger);
        }

        stos.usunZeStosu(); // usuwamy listę zmiennych obecnego bloku
        czyDeklaracjeZrobione = true; // na końcu bloku deklaracje na pewno zostaly już zrobione
    }

    public boolean isCzyDeklaracjeZrobione() {
        return czyDeklaracjeZrobione;
    }

    public void setCzyDeklaracjeZrobione(boolean czyDeklaracjeZrobione) {
        this.czyDeklaracjeZrobione = czyDeklaracjeZrobione;
    }
}

