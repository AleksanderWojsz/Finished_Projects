package Debugger;

import Stos.*;
import Instrukcje.Program;
import Wyjatki.WyjatekBlednePolecenieDebuggera;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
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

    public void wykonaj(){

        // Pobieramy wiersz ze standardowego wejścia.
        Scanner scan = new Scanner(System.in);
        String wczytanyWiersz = scan.nextLine();

        try{
            switch (pobierzZnak(wczytanyWiersz)) { // 'pobierzZnak' może rzucić wyjątek.
                case 'c':
                    polecenieContinue();
                    break;

                case 's':
                    polecenieStep(wczytanyWiersz);
                    break;

                case 'd':
                    polecenieDisplay(wczytanyWiersz);
                    wykonaj(); // Od razu ponownie uruchamiamy debugger. Nie chcemy kontynuować wykonania programu.
                    break;

                case 'm':
                    polecenieDump(wczytanyWiersz);
                    wykonaj(); // Od razu ponownie uruchamiamy debugger. Nie chcemy kontynuować wykonania programu.
                    break;

                case 'e':
                    System.exit(1);
                    break;

                default:
                    System.out.println("Błędne polecenie.");
                    wykonaj(); // ponownie uruchamiamy debugger
            }
        }
        catch (WyjatekBlednePolecenieDebuggera e){
            System.out.println(e.getMessage());
            wykonaj();
        }
    }

    private void polecenieDump(String wczytanyWiersz){
        if (program.getCzyProgramSieSkonczyl()){
            System.out.println("Dump: Program juz sie zakończył.");
            czyAktywny = true;
        }
        else { // Jeśli program się nie skończył.

            try{
                wczytanyWiersz = pobierzSciezke(wczytanyWiersz); // Może rzucić 'WyjatekBlednePolecenieDebuggera'.

                // 'Try-with-resources', żeby plik został zamknięty.
                try (PrintWriter plik = new PrintWriter(wczytanyWiersz)) { // Może rzucić 'FileNotFoundException'.
                    wykonajZrzut(plik); // Zapis do pliku.
                }
            }
            catch(FileNotFoundException e){
                System.out.println("Błąd: Nie udało się otworzyć pliku " + wczytanyWiersz);
            }
            catch (WyjatekBlednePolecenieDebuggera e){
                System.out.println(e.getMessage() + " (Dump)");
            }
        }
    }

    private void wykonajZrzut(PrintWriter plik){
        plik.println("Zadeklarowane procedury:\n" + stos.getStosProcedur().wypiszProcedury());
        plik.println("Wartościowanie:\n" + stos.getStosZmiennych().wypiszZmienne(0));
    }

    private void polecenieContinue(){
        // 'continue' daje ten sam efekt co wyłączenie debuggera.
        czyAktywny = false;

        if (program.getCzyProgramSieSkonczyl()){
            System.out.println("Continue: Program juz sie zakończył.");
            czyAktywny = true;
            wykonaj();
        }
    }

    private void polecenieStep(String wczytanyWiersz){

        try{
            liczbaKrokowDoWykonia = pobierzLiczbe(wczytanyWiersz); // Może rzucić 'WyjatekBlednePolecenieDebuggera'.

            if (liczbaKrokowDoWykonia <= 0){
                System.out.println("Step: Za mała liczba kroków");
                wykonaj();
            }
            else if (program.getCzyProgramSieSkonczyl()){
                System.out.println("Step: Program juz sie zakończył.");
                czyAktywny = true;
                wykonaj();
            }
        }
        catch (WyjatekBlednePolecenieDebuggera | NumberFormatException e){
            System.out.println(e.getMessage() + " (Step)");
        }
    }

    private void polecenieDisplay(String wczytanyWiersz){

        try{
            int liczba = pobierzLiczbe(wczytanyWiersz); // Może rzucić 'WyjatekBlednePolecenieDebuggera'.

            if (program.getCzyProgramSieSkonczyl()){
                System.out.println("Display: Program juz sie zakończył.");
                czyAktywny = true;
            }
            else {
                if (liczba < 0){
                    System.out.println("Ujemna głębokość");
                }
                else if (liczba < stos.getStosZmiennych().glebokosc()){
                    System.out.println("Wartościowanie: ");
                    System.out.println(stos.getStosZmiennych().wypiszZmienne(liczba));
                }
                else{
                    System.out.println("Za duża głębokość.");
                }
            }
        }
        catch (WyjatekBlednePolecenieDebuggera | NumberFormatException e){
            System.out.println(e.getMessage() + " (Display)");
        }
    }

    private static char pobierzZnak(String wczytanyWiersz){
        if (wczytanyWiersz.length() < 1 || (wczytanyWiersz.length() > 1 && wczytanyWiersz.charAt(1) != ' ')){
            throw new WyjatekBlednePolecenieDebuggera("Błąd: Niepoprawne parametry polecenia debuggera");
        }
        return wczytanyWiersz.charAt(0);
    }

    private static int pobierzLiczbe(String wczytanyWiersz){
        try {
            return Integer.parseInt(pobierzSciezke(wczytanyWiersz));
        } catch (NumberFormatException e) {
            throw new NumberFormatException("Błąd: Niepoprawne parametry polecenia debuggera");
        }
    }


    private static String pobierzSciezke(String wczytanyWiersz){
        // dzielimy wiersz na części względem dowolnej liczby spacji
        String[] podzielone = wczytanyWiersz.split("\\s+");

        if (podzielone.length != 2){ // Tablica powinna mieć dwa elementy
            throw new WyjatekBlednePolecenieDebuggera("Błąd: Niepoprawne parametry polecenia debuggera");
        }

        return podzielone[1];
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
