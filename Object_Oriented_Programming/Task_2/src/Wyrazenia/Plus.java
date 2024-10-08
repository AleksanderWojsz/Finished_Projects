package Wyrazenia;

import Stos.Stos;
import Wyjatki.*;

public class Plus extends Wyrazenie{

    public Plus(Wyrazenie wyrazenie1, Wyrazenie wyrazenie2) {
        super(wyrazenie1, wyrazenie2);
    }

    @Override
    public int oblicz(Stos stos) throws WyjatekNiedostepnaZmienna, WyjatekDzieleniePrzezZero {
        return getWyrazenie1().oblicz(stos) + getWyrazenie2().oblicz(stos); // oblicz może rzucić wyjątek
    }

    @Override
    public String toString(){
        return "(" + getWyrazenie1().toString() + " + " + getWyrazenie2().toString() + ")";
    }

    public static Plus of(Wyrazenie wyrazenie1, Wyrazenie wyrazenie2){
        return new Plus(wyrazenie1, wyrazenie2);
    }
}
