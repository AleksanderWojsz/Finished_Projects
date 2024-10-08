package Wyrazenia;

import Stos.Stos;

public class Literal extends Wyrazenie{
    private final int wartosc;

    public Literal(int wartosc) {
        this.wartosc = wartosc;
    }

    // Literał zawsze da sie policzyć
    @Override
    public int oblicz(Stos stos) {
        return wartosc;
    }

    @Override
    public String toString(){
        return String.valueOf(wartosc);
    }

    public static Literal of(int wartosc) {
        return new Literal(wartosc);
    }
}
