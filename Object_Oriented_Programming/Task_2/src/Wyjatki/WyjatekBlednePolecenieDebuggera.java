package Wyjatki;

public class WyjatekBlednePolecenieDebuggera extends IllegalStateException {

    public WyjatekBlednePolecenieDebuggera(String napis){
        super(napis);
    }

    public WyjatekBlednePolecenieDebuggera(){}
}
