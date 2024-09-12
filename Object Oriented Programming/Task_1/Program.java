public class Program extends Instrukcja{

    private Debugger debugger;
    private Stos stos;
    private boolean czyProgramSieSkonczyl;

    public void wykonajZDebugerrem(){
        this.stos = new Stos();
        this.debugger = new Debugger(this, stos);

        debugger.setCzyAktywny(true);
        debugger.debuggerSprawdzStan(); // uruchamiamy debuggera

        super.wykonaj(stos, debugger); // wykonujemy program

        czyProgramSieSkonczyl = true; // w tym miejscu program juz sie zakonczyl

        // mimo że program się zakończył to debugger ma działać, więc ponownie go uruchamiamy
        debugger.setCzyAktywny(true);
        debugger.wykonaj();
    }

    public void wykonaj(){
        this.stos = new Stos();
        this.debugger = new Debugger(this, stos);

        super.wykonaj(stos, debugger);
    }

    public boolean getCzyProgramSieSkonczyl() {
        return czyProgramSieSkonczyl;
    }
}
