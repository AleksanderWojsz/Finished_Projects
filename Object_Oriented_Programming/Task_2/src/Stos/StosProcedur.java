package Stos;
import Instrukcje.Procedura;
import Wyjatki.WyjatekNiedostepnaProcedura;
import java.util.ArrayList;
import java.util.List;

public class StosProcedur {

    private final List<List<Procedura>> stosProcedur;

    public StosProcedur() {
        stosProcedur = new ArrayList<>();
    }

    public void dodajNaStos(List<Procedura> listaProcedur) {
        stosProcedur.add(listaProcedur);
    }

    public void usunZeStosu() {
        stosProcedur.remove(stosProcedur.size() - 1);
    }

    // Szuka zmiennej o podanej nazwie w listach na stosie, jeśli takiej nie ma to rzuca wyjątek.
    public Procedura znajdzProcedure(String nazwa) {
        for (int i = stosProcedur.size() - 1; i >= 0; i--) {
            for (Procedura procedura : stosProcedur.get(i)) {
                if (procedura.getNazwa().equals(nazwa)) {
                    return procedura;
                }
            }
        }
        throw new WyjatekNiedostepnaProcedura("Niedostepna procedura: " + nazwa);
    }

    // Daje, w postaci napisu, wszystkie procedury, które są na stosie.
    public String wypiszProcedury(){
        StringBuilder wynik = new StringBuilder();

        for (List<Procedura> listaProcedur : stosProcedur) {
            for (Procedura procedura : listaProcedur) {
                wynik.append(procedura.toString()).append("\n");
            }
        }

        return wynik.toString();
    }
}
