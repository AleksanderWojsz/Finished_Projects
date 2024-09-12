package Buildery;
import Instrukcje.*;

public class BlockBuilder extends Builder<Blok> {

    public BlockBuilder(){
        super.setStruktura(new Blok()); // Tworzymy nowy blok, do którego Builder będzie dodawał kolejne instrukcje.
    }
}
