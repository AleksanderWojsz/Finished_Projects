public class Modulo extends Wyrazenie{

    public Modulo(Wyrazenie wyrazenie1, Wyrazenie wyrazenie2) {
        super(wyrazenie1, wyrazenie2);
    }

    @Override
    public int oblicz(Stos stos) throws WyjatekNiedostepnaZmienna, WyjatekDzieleniePrzezZero{

        if (getWyrazenie2().oblicz(stos) == 0)
        {
            throw new WyjatekDzieleniePrzezZero("Dzielenie przez zero");
        }
        else {
            return getWyrazenie1().oblicz(stos) % getWyrazenie2().oblicz(stos);
        }
    }

    @Override
    public String toString(){
        return "(" + getWyrazenie1().toString() + " % " + getWyrazenie2().toString() + ")";
    }
}