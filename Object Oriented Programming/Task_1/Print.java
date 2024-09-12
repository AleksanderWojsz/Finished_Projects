public class Print extends Instrukcja {
    private final Wyrazenie wyrazenie;

    public Print(Wyrazenie wyrazenie) {
        this.wyrazenie = wyrazenie;
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger(this.toString(), debugger);

        try {
            System.out.println(wyrazenie.oblicz(stos)); // oblicz może rzucić błąd
        }
        catch (WyjatekNiedostepnaZmienna | WyjatekDzieleniePrzezZero e) {
            obsluzBlad(e, stos);
        }
    }

    @Override
    public String toString(){
        return "print " +  wyrazenie.toString();
    }
}


