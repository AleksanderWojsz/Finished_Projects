public class KoniecBloku extends Instrukcja{

    @Override
    protected void wykonaj(Stos stos, Debugger debugger){
        obsluzDebugger("Wyjście z bloku", debugger);
    }
}
