package Instrukcje;
import Stos.*;
import Debugger.Debugger;

public class Program extends Blok {

    private Debugger debugger;
    private Stos stos;
    private boolean czyProgramSieSkonczyl;

    public void wykonajZDebugerrem(){
        this.stos = new Stos();
        this.debugger = new Debugger(this, stos);

        debugger.setCzyAktywny(true);
        debugger.debuggerSprawdzStan(); // uruchamiamy debuggera

        wykonaj(stos, debugger); // wykonujemy program

        czyProgramSieSkonczyl = true; // w tym miejscu program juz sie zakonczyl

        // mimo że program się zakończył to debugger ma działać, więc ponownie go uruchamiamy
        debugger.setCzyAktywny(true);
        debugger.wykonaj();
    }

    public void wykonaj(){
        this.stos = new Stos();
        this.debugger = new Debugger(this, stos);

        wykonaj(stos, debugger);
    }

    public boolean getCzyProgramSieSkonczyl() {
        return czyProgramSieSkonczyl;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){
        obsluzDebugger("", debugger); // żeby debugger zatrzymał się przed pierwszą instrukcją
        obsluzDebugger("Wejscie do bloku", debugger);

        // chcemy żeby blok w pierwszej kolejności brał pod uwagę "aktualne" i zadeklarowane w nim zmienne
        stos.getStosZmiennych().dodajNaStos(getListaZmiennych());
        stos.getStosProcedur().dodajNaStos(getListaProcedur()); // dodajemy liste procedur obecnego bloku

        // wykonujemy instrukcje które są w bloku
        for (int i = 0; i < getListaInstrukcji().size(); i++) {
            getListaInstrukcji().get(i).ustawBlok(this);
            getListaInstrukcji().get(i).wykonaj(stos, debugger);
        }

        // program, po zakończeniu swoich instrukcji wypisuje jeszcze obecne wartościowanie
        System.out.println("\nWartościowanie: ");
        System.out.println(stos.getStosZmiennych().wypiszZmienne(0));

        stos.getStosProcedur().usunZeStosu();
        stos.getStosZmiennych().usunZeStosu(); // usuwamy listę zmiennych obecnego bloku
        setCzyDeklaracjeZrobione(true); // na końcu bloku deklaracje na pewno zostaly już zrobione
    }
}



