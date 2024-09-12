package Wyjatki;

public class WyjatekNiepoprawnaNazwa extends IllegalStateException {

    public WyjatekNiepoprawnaNazwa(String napis){
        super(napis);
    }
}
