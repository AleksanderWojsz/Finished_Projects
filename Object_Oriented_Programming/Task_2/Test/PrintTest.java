import static org.junit.jupiter.api.Assertions.*;
import Wyrazenia.Plus;
import org.junit.jupiter.api.*;
import java.io.*;
import Buildery.*;
import Instrukcje.*;
import Wyrazenia.Literal;

public class PrintTest {

    private PrintStream pierwotneOut;
    private ByteArrayOutputStream byteArrayOutputStream;

    @BeforeEach
    void zmienStrumien(){
        pierwotneOut = System.out; // Zapisujemy pierwotne System.out
        byteArrayOutputStream = new ByteArrayOutputStream(); // Nowy strumien wyjsciowy.
        System.setOut(new PrintStream(byteArrayOutputStream)); // Zmiana System.out
    }

    @AfterEach
    void przywrocStrumien(){
        System.setOut(pierwotneOut); // Przywrocenie System.out
    }

    @Test
    void printLiteralTest() {

        Program program = new ProgramBuilder()
            .wypisz(Literal.of(5))
            .build();
        program.wykonaj();

        assertEquals('5', byteArrayOutputStream.toString().charAt(0));
    }

    @Test
    void printWyrazenieTest() {

        Program program = new ProgramBuilder()
            .wypisz(Plus.of(Literal.of(3), Literal.of(7)))
            .build();
        program.wykonaj();

        assertEquals("10", byteArrayOutputStream.toString().substring(0, 2));
    }
}
