public abstract class Wyrazenie{

    private Wyrazenie wyrazenie1;
    private Wyrazenie wyrazenie2;

    public Wyrazenie(Wyrazenie wyrazenie1, Wyrazenie wyrazenie2) {
        this.wyrazenie1 = wyrazenie1;
        this.wyrazenie2 = wyrazenie2;
    }

    public Wyrazenie(){} // zmienna i literał nie dostają wyrażeń więc mają inny konstruktor

    public abstract int oblicz(Stos stos);

    public abstract String toString();

    public Wyrazenie getWyrazenie1() {
        return wyrazenie1;
    }

    public Wyrazenie getWyrazenie2() {
        return wyrazenie2;
    }
}
