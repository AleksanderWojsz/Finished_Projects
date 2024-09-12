package Wyjatki;

public class WyjatekNiepoprawneNazwyArgumentow extends IllegalStateException {

    public WyjatekNiepoprawneNazwyArgumentow(String napis){
        super(napis);
    }
}
