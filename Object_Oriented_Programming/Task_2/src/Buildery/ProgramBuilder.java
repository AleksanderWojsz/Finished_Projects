package Buildery;
import Instrukcje.*;

public class ProgramBuilder extends Builder<Program> {

    public ProgramBuilder(){
        super.setStruktura(new Program()); // Tworzymy nowy program, do którego Builder będzie dodawał kolejne instrukcje.
    }
}
