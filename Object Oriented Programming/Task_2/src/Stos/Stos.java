package Stos;

public class Stos {

    // Stos realizujemy jako listę list zmiennych lub procedur.
    // Im lista jest wyżej na stosie, tym zmienne/procedury na niej są bardziej zagnieżdżone w programie.
    private final StosProcedur stosProcedur;
    private final StosZmiennych stosZmiennych;

    public Stos() {
        this.stosProcedur = new StosProcedur();
        this.stosZmiennych = new StosZmiennych();
    }

    public StosProcedur getStosProcedur(){
        return stosProcedur;
    }

    public StosZmiennych getStosZmiennych(){
        return stosZmiennych;
    }
}
