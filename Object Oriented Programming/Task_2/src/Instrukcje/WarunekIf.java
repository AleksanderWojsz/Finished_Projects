package Instrukcje;

import Debugger.Debugger;
import Stos.Stos;
import Wyrazenia.Wyrazenie;
import Wyjatki.*;
import java.util.ArrayList;
import java.util.List;

public class WarunekIf extends Instrukcja{
    private final Wyrazenie wyrazenie1;
    private final Wyrazenie wyrazenie2;
    private final String porownanie;
    private final List<Instrukcja> elseListaInstrukcji;

    public WarunekIf(Wyrazenie wyrazenie1, String porownanie, Wyrazenie wyrazenie2) {
        this.wyrazenie1 = wyrazenie1;
        this.wyrazenie2 = wyrazenie2;
        this.porownanie = porownanie;
        elseListaInstrukcji = new ArrayList<>();
    }

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){

        obsluzDebugger(this.toString(), debugger);

        try {
            // sprawdzamy czy warunek 'if' jest spełniony
            if ((porownanie.equals("=") && wyrazenie1.oblicz(stos) == wyrazenie2.oblicz(stos)) ||
                    (porownanie.equals("<>") && wyrazenie1.oblicz(stos) != wyrazenie2.oblicz(stos)) ||
                    (porownanie.equals("<") && wyrazenie1.oblicz(stos) < wyrazenie2.oblicz(stos)) ||
                    (porownanie.equals(">") && wyrazenie1.oblicz(stos) > wyrazenie2.oblicz(stos)) ||
                    (porownanie.equals("<=") && wyrazenie1.oblicz(stos) <= wyrazenie2.oblicz(stos)) ||
                    (porownanie.equals(">=") && wyrazenie1.oblicz(stos) >= wyrazenie2.oblicz(stos))){

                super.wykonaj(stos, debugger); // wykonanie warunku 'if'
            }
            else if (elseListaInstrukcji.size() > 0){ // wykonanie 'else'
                wykonajDlaElse(stos, debugger);
            }
        }
        catch (WyjatekNiedostepnaZmienna | WyjatekDzieleniePrzezZero e) {
            obsluzBlad(e, stos);
        }
    }

    public void dodajInstrukcjeElse(Instrukcja instrukcja){
        elseListaInstrukcji.add(instrukcja);
    }

    private void wykonajDlaElse(Stos stos, Debugger debugger){

        obsluzDebugger("else (if)", debugger);

        for (Instrukcja instrukcja : elseListaInstrukcji) {
            instrukcja.wykonaj(stos, debugger);
        }
    }

    @Override
    public String toString(){
        return "if " + wyrazenie1.toString() + " " + porownanie + " " + wyrazenie2.toString();
    }
}