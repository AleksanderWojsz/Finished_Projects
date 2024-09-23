public class Main {
    public static void main(String[] args) {

        // Tworzenie potrzebnych obiektów
        Program program = new Program();

        BlokGlowny blokGlwowny = new BlokGlowny();
        Blok blok2 = new Blok();
        KoniecBloku koniecBloku = new KoniecBloku(); // każdy blok może mieć ten sam koniec bloku

        Zmienna zmiennaN = new Zmienna('n');
        Deklaracja deklaracjaN = new Deklaracja('n', new Literal(30));
        Zmienna zmiennaP = new Zmienna('p');
        Deklaracja deklaracjaP = new Deklaracja('p', new Literal(1));
        Zmienna zmiennaK = new Zmienna('k');
        Zmienna zmiennaI = new Zmienna('i');
        Literal literal0 = new Literal(0);
        Literal literal1 = new Literal(1);
        Literal literal2 = new Literal(2);

        Plus plus1 = new Plus(zmiennaK, literal2);
        Plus plus2 = new Plus(zmiennaI, literal2);
        Minus minus1 = new Minus(zmiennaN, literal1);
        Minus minus2 = new Minus(zmiennaK, literal2);
        Modulo modulo1 = new Modulo(zmiennaK, zmiennaI);

        Przypisz przypisz1 = new Przypisz('k', plus1);
        Przypisz przypisz2 = new Przypisz('i', plus2);
        Przypisz przypisz3 = new Przypisz('p', literal0);

        WarunekIf warunekIf1 = new WarunekIf(modulo1, "=", literal0);
        WarunekIf warunekIf2 = new WarunekIf(zmiennaP, "=", literal1);

        PetlaFor petlaFor1 = new PetlaFor('k', minus1);
        PetlaFor petlaFor2 = new PetlaFor('i', minus2);

        Print print1 = new Print(zmiennaK);


        // Połącznie stworzonych obiektów w program
        program.dodajInstrukcje(blokGlwowny);

        blokGlwowny.dodajInstrukcje(deklaracjaN);
        blokGlwowny.dodajInstrukcje(petlaFor1);
        blokGlwowny.dodajInstrukcje(koniecBloku);

        petlaFor1.dodajInstrukcje(blok2);

        blok2.dodajInstrukcje(deklaracjaP);
        blok2.dodajInstrukcje(przypisz1);
        blok2.dodajInstrukcje(petlaFor2);
        blok2.dodajInstrukcje(warunekIf2);
        blok2.dodajInstrukcje(koniecBloku);

        petlaFor2.dodajInstrukcje(przypisz2);
        petlaFor2.dodajInstrukcje(warunekIf1);

        warunekIf1.dodajInstrukcje(przypisz3);
        warunekIf2.dodajInstrukcje(print1);

        program.wykonaj();

        // Uruchomienie programu z debuggerem:
        // program.wykonajZDebugerrem();

    }
}
