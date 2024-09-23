package Wyrazenia;

import Wyjatki.*;
import Stos.*;

public class Dzielenie extends Wyrazenie{

    public Dzielenie(Wyrazenie wyrazenie1, Wyrazenie wyrazenie2) {
        super(wyrazenie1, wyrazenie2);
    }

    @Override
    public int oblicz(Stos stos) throws WyjatekNiedostepnaZmienna, WyjatekDzieleniePrzezZero {

        if (getWyrazenie2().oblicz(stos) == 0) {
            throw new WyjatekDzieleniePrzezZero("Wyrazenia.Dzielenie przez zero");
        }
        else {
            return getWyrazenie1().oblicz(stos) / getWyrazenie2().oblicz(stos); // oblicz może rzucić wyjątek
        }
    }

    @Override
    public String toString(){
        return "(" + getWyrazenie1().toString() + " / " + getWyrazenie2().toString() + ")";
    }

    public static Dzielenie of(Wyrazenie wyrazenie1, Wyrazenie wyrazenie2){
        return new Dzielenie(wyrazenie1, wyrazenie2);
    }
}