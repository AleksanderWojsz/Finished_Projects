import Stos.Stos;
import Wyjatki.WyjatekDzieleniePrzezZero;
import Wyrazenia.*;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

public class WyrazeniaTest {

    @Test
    void dzielenieTest(){
        Dzielenie dzielenie = Dzielenie.of(Literal.of(30), Literal.of(2));
        assertEquals(15, dzielenie.oblicz(new Stos()));
    }

    @Test
    void dzieleniePrzezZeroTest(){
        Dzielenie dzielenie = Dzielenie.of(Literal.of(453), Literal.of(0));
        assertThrows(WyjatekDzieleniePrzezZero.class, () -> dzielenie.oblicz(new Stos()));
    }

    @Test
    void literalTest(){
        Literal literal = Literal.of(10);
        assertEquals(10, literal.oblicz(new Stos()));
    }

    @Test
    void minusTest(){
        Minus minus = Minus.of(Literal.of(123), Literal.of(23));
        assertEquals(100, minus.oblicz(new Stos()));
    }

    @Test
    void mnozenieTest(){
        Mnozenie mnozenie = Mnozenie.of(Literal.of(3), Literal.of(9));
        assertEquals(27, mnozenie.oblicz(new Stos()));
    }

    @Test
    void moduloTest(){
        Modulo modulo = Modulo.of(Literal.of(18), Literal.of(5));
        assertEquals(3, modulo.oblicz(new Stos()));
    }

    @Test
    void plusTest(){
        Plus plus = Plus.of(Literal.of(18), Literal.of(5));
        assertEquals(23, plus.oblicz(new Stos()));
    }

    @Test
    void zmiennaTest(){
        Zmienna zmienna = Zmienna.of('a');
        assertEquals('a', zmienna.getNazwa());
    }
}
