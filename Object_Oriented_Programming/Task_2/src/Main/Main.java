package Main;

import Buildery.*;
import Wyrazenia.*;
import java.util.List;

public class Main {
    public static void main(String[] args) {

        // W programie występuje dynamiczne wiązanie zmiennych.

        var program = new ProgramBuilder()
            .nowaZmienna('x', Literal.of(101))
            .nowaZmienna('y', Literal.of(1))
            .nowaProcedura("out", List.of('a'), new BlockBuilder()
                .wypisz(Plus.of(Zmienna.of('a'), Zmienna.of('x')))
                .build()
            )
            .przypisz('x', Minus.of(Zmienna.of('x'), Zmienna.of('y')))
            .wywolaj("out", List.of(Zmienna.of('x')))
            .wywolaj("out", List.of(Literal.of(100)))
            .nowyBlok(new BlockBuilder()
                .nowaZmienna('x', Literal.of(10))
                .wywolaj("out", List.of(Literal.of(100)))
                .build()
            )
            .build();

        program.wykonaj();

        // Uruchomienie z debuggerem.
        // program.wykonajZDebugerrem();
    }
}
