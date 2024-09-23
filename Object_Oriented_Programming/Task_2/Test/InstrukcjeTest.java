import Buildery.*;
import Instrukcje.*;
import Wyrazenia.*;
import org.junit.jupiter.api.Test;
import java.util.List;
import static org.junit.jupiter.api.Assertions.*;

// Testy z wykorzystaniem klasy 'BlokTestowy', która pozwala
// na odczytanie wartości konkretnych zmiennych po zakończeniu wykonania programu.
public class InstrukcjeTest {

    @Test
    void deklaracjaTest(){ // Czy zmienna istnieje i ma zadeklarowaną wartośc.

        BlokTestowy blokTestowy = new BlokTestowy('a', 0);

        Program program = new ProgramBuilder()
            .nowaZmienna('a', Literal.of(5))
            .nowyBlok(blokTestowy).build();
        program.wykonaj();

        assertEquals(5, blokTestowy.dajWynik());
    }

    @Test
    void przypisanieTest(){ // Sprawdzenie, czy zmienna faktycznie ma przypisaną wartość.

        BlokTestowy blokTestowy = new BlokTestowy('a', 0);

        Program program = new ProgramBuilder()
                .nowaZmienna('a', Literal.of(5))
                .przypisz('a', Literal.of(10)) // Teraz powinno być 'a = 10'.
                .nowyBlok(blokTestowy).build();
        program.wykonaj();

        assertEquals(10, blokTestowy.dajWynik());
    }

    @Test
    void widocznosciBlokTest(){ // Sprawdzenie, czy deklaracje w bloku mają dobrą widoczność.

        BlokTestowy blokTestowy = new BlokTestowy('a', 0);

        Program program = new ProgramBuilder()
            .nowaZmienna('a', Literal.of(1))
            .nowyBlok( new BlockBuilder()
                    .nowaZmienna('a', Literal.of(2))
                    .build()
            )
            .nowyBlok(blokTestowy).build(); // W tym miejscu powinno być widoczne 'a = 1'.

        program.wykonaj();

        assertEquals(1, blokTestowy.dajWynik());
    }

    @Test
    void warunekIfTest(){

        BlokTestowy blokTestowy = new BlokTestowy('a', 0);

        Program program = new ProgramBuilder()
            // Warunek if powinien być spełniony i ustawić 'a' na '1'.
            .warunekIf(Literal.of(5), "<", Literal.of(6), new BlockBuilder()
                .nowaZmienna('a', Literal.of(1))
                .nowyBlok(blokTestowy)
                .build(), new BlockBuilder().build() // Blok 'else' warunku if jest pusty.
            )
            .build();
        program.wykonaj();

        assertEquals(1, blokTestowy.dajWynik());
    }

    @Test
    void petlaForTest(){ // Czy na koniec pętli od 0 do 9, iterowana zmienna jest równa 9.

        BlokTestowy blokTestowy = new BlokTestowy('i', 0);

        Program program = new ProgramBuilder()
            .petlaFor('i', Literal.of(10), blokTestowy)
            .build();
        program.wykonaj();

        assertEquals(9, blokTestowy.dajWynik());
    }

    @Test
    void proceduraTest(){ // Czy procedura wykona oczekiwaną deklarację.

        BlokTestowy blokTestowy = new BlokTestowy('a', 0);

        Program program = new ProgramBuilder()
            .nowaProcedura("proc", List.of(), new BlockBuilder()
                .nowaZmienna('a', Literal.of(5))
                .nowyBlok(blokTestowy)
                .build()
            )
            .wywolaj("proc", List.of())
            .build();

        program.wykonaj();

        assertEquals(5, blokTestowy.dajWynik());
    }

    @Test
    void proceduraRekurencjaTest() { // Wypisuje kolejne liczby naturalne od 1 do 10.

        BlokTestowy blokTestowy = new BlokTestowy('x', 0);

        var program = new ProgramBuilder()
            .nowaProcedura("out", List.of('x'), new BlockBuilder()
                .warunekIf(Zmienna.of('x'), "<", Literal.of(10), new BlockBuilder()
                    .przypisz('x', Plus.of(Zmienna.of('x'), Literal.of(1)))
                    .wypisz(Zmienna.of('x'))
                    .nowyBlok(blokTestowy)
                    .wywolaj("out", List.of(Zmienna.of('x')))
                    .build(),
                     new BlockBuilder().build() // Blok 'else' jest pusty.
                )
                .build()
            )
            .wywolaj("out", List.of(Literal.of(0)))
            .build();
        program.wykonaj();

        assertEquals(10, blokTestowy.dajWynik());
    }

    @Test
    void proceduraPowinnaBycPrzeslonieta() {

        BlokTestowy blokTestowy = new BlokTestowy('a', 0);

        var program = new ProgramBuilder()
            .nowaZmienna('a', Literal.of(0))
            .nowaProcedura("proc", List.of(), new BlockBuilder()
                .przypisz('a', Literal.of(1))
                .build()
            )
            .nowyBlok(new BlockBuilder()
                .nowaProcedura("proc", List.of(), new BlockBuilder()
                    .przypisz('a', Literal.of(2))
                    .build()
                )
                .wywolaj("proc", List.of()) // Powinna wywołać się procedura ustawiająca 'a = 2'.
                .nowyBlok(blokTestowy)
                .build())
            .build();
        program.wykonaj();

        assertEquals(2, blokTestowy.dajWynik());
    }
}
