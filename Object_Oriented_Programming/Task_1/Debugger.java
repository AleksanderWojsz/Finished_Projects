import java.util.Scanner;

public class Debugger {

    private boolean czyAktywny;
    private int liczbaKrokowDoWykonia;
    private final Stos stos;
    private final Program program;

    public Debugger(Program program, Stos stos){
        this.stos = stos;
        this.program = program;
    }

    protected void wykonaj(){
        int liczba;

        // pobieramy wiersz ze standardowego wejscia
        Scanner scan = new Scanner(System.in);
        String wiersz = scan.nextLine();

        switch (pobierzZnak(wiersz)) {
            case 'c':
                // 'continue' daje ten sam efekt co wyłączenie debuggera
                czyAktywny = false;

                if (program.getCzyProgramSieSkonczyl()){
                    System.out.println("Continue: Program juz sie zakonczyl.");
                    czyAktywny = true;
                    wykonaj();
                }

                break;
            case 's':

                liczba = pobierzLiczbe(wiersz);
                this.liczbaKrokowDoWykonia = liczba;

                if (liczba <= 0){
                    System.out.println("Step: Za mała liczba kroków");
                    wykonaj();
                }
                else if (program.getCzyProgramSieSkonczyl()){
                    System.out.println("Step: Program juz sie zakonczyl.");
                    czyAktywny = true;
                    wykonaj();
                }

                break;
            case 'd':

                liczba = pobierzLiczbe(wiersz);

                if (program.getCzyProgramSieSkonczyl()){
                    System.out.println("Display: Program juz sie zakonczyl.");
                    czyAktywny = true;
                }
                else {
                    if (liczba < 0){
                        System.out.println("Ujemna głębokość");
                    }
                    else if (liczba < stos.glebokosc()){
                        System.out.println("Wartościowanie: ");
                        stos.wypiszZmienne(liczba);
                    }
                    else{
                        System.out.println("Za duza glebokosc.");
                    }
                }

                wykonaj(); // od razu ponownie uruchamiamy debugger. Nie chemy kontynoować programu

                break;
            case 'e':

                System.exit(1);

                break;
            default:
                System.out.println("Błędne polecenie.");
                wykonaj(); // ponownie uruchamiamy debugger
        }
    }

    public static char pobierzZnak(String wiersz){
        return wiersz.charAt(0);
    }

    public static int pobierzLiczbe(String wiersz){
        // dzielimy wiersz na części względem dowolnej liczby spacji
        String[] podzielone = wiersz.split("\\s+");

        return Integer.parseInt(podzielone[1]);

    }

    public void debuggerSprawdzStan(){
        if (czyAktywny){
            if (liczbaKrokowDoWykonia <= 0){ // jesli skonczyly sie kroki do wykonania (step sie zatrzymal)
                wykonaj();
            }
        }
    }

    public void zmniejszLiczbeKrokow(){
        liczbaKrokowDoWykonia--;
    }

    public int getLiczbaKrokowDoWykonia() {
        return liczbaKrokowDoWykonia;
    }

    public boolean isCzyAktywny() {
        return czyAktywny;
    }

    public void setCzyAktywny(boolean czyAktywny) {
        this.czyAktywny = czyAktywny;
    }
}
