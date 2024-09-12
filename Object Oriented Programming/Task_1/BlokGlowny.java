public class BlokGlowny extends Blok{

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger("Wejscie do bloku głównego", debugger);

        // chcemy żeby blok w pierwszej kolejności brał pod uwagę "aktualne" i zadeklarowane w nim zmienne
        stos.dodajNaStos(getListaZmiennych());

        // wykonujemy instrukcje które są w bloku
        for (int i = 0; i < getListaInstrukcji().size(); i++) {
            getListaInstrukcji().get(i).ustawBlok(this);
            getListaInstrukcji().get(i).wykonaj(stos, debugger);
        }

        // blok głowny, po zakończeniu swoich instrukcji wypisuje jeszcze obecne wartościowanie
        System.out.println("\nWartościowanie: ");
        stos.wypiszZmienne(0);

        stos.usunZeStosu(); // usuwamy listę zmiennych obecnego bloku
        setCzyDeklaracjeZrobione(true); // na końcu bloku deklaracje na pewno zostaly już zrobione
    }
}
