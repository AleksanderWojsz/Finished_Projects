package cp2023.solution;

import cp2023.base.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Semaphore;
import cp2023.exceptions.TransferException;

public class Storage implements StorageSystem {

    protected HashMap<DeviceId, Urzadzenie> urzadzenia;

    public Storage(HashMap<DeviceId, Urzadzenie> urzadzenia) {
        this.urzadzenia = urzadzenia;
    }

    public static boolean czyDodawanie(ComponentTransfer transfer) {
        return transfer.getDestinationDeviceId() != null && transfer.getSourceDeviceId() == null;
    }

    public static boolean czyUsuwanie(ComponentTransfer transfer) {
        return transfer.getDestinationDeviceId() == null && transfer.getSourceDeviceId() != null;
    }

    public static boolean czyPrzenoszenie(ComponentTransfer transfer) {
        return transfer.getDestinationDeviceId() != null && transfer.getSourceDeviceId() != null;
    }

    public void dodajPolaczenieWGrafie(ComponentTransfer transfer) {
        urzadzenia.get(transfer.getSourceDeviceId()).listaNiewykonanychTransferowWyjsciowych.add(transfer);
    }

    public void usunPolaczenieZGrafu(ComponentTransfer transfer) {
        if (urzadzenia.containsKey(transfer.getSourceDeviceId())) {
            urzadzenia.get(transfer.getSourceDeviceId()).listaNiewykonanychTransferowWyjsciowych.remove(transfer);
        }
    }

    public boolean szukajSciezki(ComponentTransfer transfer, Urzadzenie zrodlo, Urzadzenie cel, Set<DeviceId> odwiedzone, LinkedList<ComponentTransfer> sciezkaTransferow) {
        odwiedzone.add(zrodlo.idUrzadzenia);
        sciezkaTransferow.add(transfer);

        if (zrodlo.equals(cel)) {
            return true;
        }

        List<ComponentTransfer> listaNiewykonanychTransferowWyjsciowych = new LinkedList<>(zrodlo.listaNiewykonanychTransferowWyjsciowych);
        for (int i = 0; i < listaNiewykonanychTransferowWyjsciowych.size(); i++) {
            ComponentTransfer transferWyjsciowy = listaNiewykonanychTransferowWyjsciowych.get(i);
            Urzadzenie celTransferu = urzadzenia.get(transferWyjsciowy.getDestinationDeviceId());
            if (!odwiedzone.contains(celTransferu.idUrzadzenia)) {
                if (szukajSciezki(transferWyjsciowy, celTransferu, cel, odwiedzone, sciezkaTransferow)) {
                    return true;
                }
            }
        }

        sciezkaTransferow.removeLast();
        return false;
    }

    // Sprawdza, czy po dodaniu krawedzi w grafie powstanie cykl. Jesli tak to jako wynik daje transfery w tym cyklu lub null w.p.p.
    public LinkedList<ComponentTransfer> czyPoDodaniuBedzieCykl(ComponentTransfer transfer) {
        LinkedList<ComponentTransfer> sciezkaTransferow = new LinkedList<>();

        // Sprawdzamy, czy jest sciezka od celu do zrodla transferu, bo wtedy, po dodaniu transferu powstanie cykl.
        Urzadzenie cel = urzadzenia.get(transfer.getSourceDeviceId());
        Urzadzenie zrodlo = urzadzenia.get(transfer.getDestinationDeviceId());

        Set<DeviceId> odwiedzone = new HashSet<>();

        return szukajSciezki(transfer, zrodlo, cel, odwiedzone, sciezkaTransferow) ? sciezkaTransferow : null;
    }

    // Kazdy transfer ma swoj semafor, na ktorym moze czekac na swoja kolej w szukaniu cyklu.
    Map<ComponentId, Semaphore> tylkoJedenMozeSzukacCyklu = new ConcurrentHashMap<>();

    // Potrzebne do czekania na 'prepare' swojego nastepnika w cyklu.
    Map<ComponentTransfer, Semaphore> czekanieNaPrepare = new ConcurrentHashMap<>();
    Map<ComponentTransfer, ComponentTransfer> poprzednicy = new ConcurrentHashMap<>();

    // Pamieta transfery dla ktorych liczba miejsc na urzadzeniu docelowym zostala zmniejszona.
    Set<ComponentTransfer> miejsceJuzZmniejszone = Collections.synchronizedSet(new HashSet<>());

    // Wszystkie aktywne transfery w systemie.
    HashSet<ComponentId> aktywneTransfery = new HashSet<>();

    List<ComponentTransfer> cykl = null;
    Set<ComponentTransfer> cyklKopia = Collections.synchronizedSet(new HashSet<>());

    Semaphore mutex = new Semaphore(1);
    Semaphore mutexCykl = new Semaphore(1);
    Semaphore mutexLiczenieAktywnychTransferow = new Semaphore(1);

    @Override
    public void execute(ComponentTransfer transfer) throws TransferException {
        Para para = new Para(transfer);

        Urzadzenie zrodlo = urzadzenia.get(transfer.getSourceDeviceId());
        Urzadzenie cel = urzadzenia.get(transfer.getDestinationDeviceId());
        ComponentId componentId = transfer.getComponentId();

        Wyjatki.czyParametryTransferuPoprawne(transfer);
        Wyjatki.czyCelIZrodloIstnieja(transfer, urzadzenia);

        try {
            mutexLiczenieAktywnychTransferow.acquire();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }

        // Podnosi 'mutexLiczenieAktywnychTransferow' i modyfikuje 'aktywneTransfery'.
        Wyjatki.czyKomponentJeszczeTransferowany(componentId, aktywneTransfery, mutexLiczenieAktywnychTransferow);


        if (czyUsuwanie(transfer)) {
            Wyjatki.czyKomponentNaWskazanymUrzadzeniu(componentId, zrodlo);

            try {
                mutex.acquire();
            } catch (InterruptedException e) {
                throw new RuntimeException("panic: unexpected thread interruption");
            }
            zrodlo.usunKomponentZPamieci(componentId);
            zrodlo.wznowPierwszyCzekajacy();
            mutex.release();
            transfer.prepare();
            transfer.perform();
        }
        else if (czyDodawanie(transfer)) {
            Wyjatki.czyDodawanyKomponentJuzWSystmie(componentId, urzadzenia);

            try {
                mutex.acquire();
            } catch (InterruptedException e) {
                throw new RuntimeException("panic: unexpected thread interruption");
            }
            if (!cel.czyWolneMiejsce()) {
                // Dodajemy obecny transfer jako oczekujacy na miejce na docelowym urzadzeniu.
                cel.czekajaceNaMiejsce.add(para);
                mutex.release();
                try {
                    para.semafor.acquire();
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }

                // Tu juz jest wolne miejsce.

                transfer.prepare();
                transfer.perform();
                cel.zajeteMiejsca.add(componentId);
                miejsceJuzZmniejszone.remove(transfer);

            } else {
                cel.dodajKomponentDoPamieci(componentId);
                mutex.release();
                transfer.prepare();
                transfer.perform();
            }
        }
        else if (czyPrzenoszenie(transfer)) {
            Wyjatki.czyKomponentNaWskazanymUrzadzeniu(componentId, zrodlo);
            Wyjatki.czyKomponentPotrzebujeTransferu(transfer);

            try {
                mutex.acquire();
            } catch (InterruptedException e) {
                throw new RuntimeException("panic: unexpected thread interruption");
            }
            if (!cel.czyWolneMiejsce()) {
                mutex.release();

                // Dodajemy transfer do oczekujacych, zeby w sytuacji gdzie transfer bedzie w cyklu
                // mozna bylo go znalezc i podniesc mu semafor.
                cel.czekajaceNaMiejsce.add(para);
                dodajPolaczenieWGrafie(transfer);

                try {
                    mutexCykl.acquire();
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }
                if ((tylkoJedenMozeSzukacCyklu.isEmpty() && (cyklKopia.isEmpty() || cyklKopia.contains(transfer))) ||
                        cyklKopia.contains(transfer)) {
                    tylkoJedenMozeSzukacCyklu.put(componentId, new Semaphore(1));
                }
                else {
                    tylkoJedenMozeSzukacCyklu.put(componentId, new Semaphore(0));
                }
                mutexCykl.release();

                try {
                    tylkoJedenMozeSzukacCyklu.get(componentId).acquire();
                    mutexCykl.acquire();
                    mutex.acquire();
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }

                /*
                 W dowolnym momencie tylko jeden transfer moze szukac cyklu.
                 Pozwolenie na szukanie jest oddawane dopiero, wtedy
                 gdy cyklu nie ma lub wszystkie transfery w znalezionym cyklu zostana wykonane.
                 Jesli element nie jest juz w cyklu i w urzadzeniu docelowym
                 caly czas nie ma miejsca, to szukamy cyklu.
                */
                if (cykl == null && !cel.czyWolneMiejsce() && !miejsceJuzZmniejszone.contains(transfer)) { // Nie szukamy cyklu, jak mamy rezerwacje.
                    cykl = czyPoDodaniuBedzieCykl(transfer);
                    if (cykl != null) {
                        cykl = Collections.synchronizedList(cykl);
                    }
                }
                else if (cel.czyWolneMiejsce()){
                    cel.liczbaWolnychMiejsc.decrementAndGet();
                    miejsceJuzZmniejszone.add(transfer);
                    usunPolaczenieZGrafu(transfer);
                }
                mutex.release();
                mutexCykl.release();

                if (cykl != null && !cyklKopia.contains(transfer)) {
                    // Kazdy transfer bedzie czekal na 'prepare' swojego poprzednika w cyklu.
                    // Kazdemu chcemy dac miejsce do czekania zanim go obudzimy.
                    for (ComponentTransfer transferWCyklu : cykl) {
                        czekanieNaPrepare.put(transferWCyklu, new Semaphore(0));
                        poprzednicy.put(transferWCyklu, (transferWCyklu != cykl.get(0) ?
                                cykl.get(cykl.indexOf(transferWCyklu) - 1) :
                                cykl.get(cykl.size() - 1)));
                    }
                    cyklKopia.addAll(cykl);

                    for (ComponentTransfer transferWCyklu : cykl) {
                        try {
                            mutexCykl.acquire();
                        } catch (InterruptedException e) {
                            throw new RuntimeException("panic: unexpected thread interruption");
                        }

                        // Chcemy zeby transfery bedace w cyklu mogly ominac czekanie na szukanie cyklu, wiec podnosimy ich semafory.
                        if (tylkoJedenMozeSzukacCyklu.containsKey(transferWCyklu.getComponentId())) {
                            tylkoJedenMozeSzukacCyklu.get(transferWCyklu.getComponentId()).release();
                        }

                        urzadzenia.get(transferWCyklu.getDestinationDeviceId()).wznowWskazanyCzekajacy(transferWCyklu);
                        mutexCykl.release();
                    }
                }
                else if (!cyklKopia.contains(transfer)) { // Nie ma cyklu.

                    try {
                        mutexCykl.acquire();
                    } catch (InterruptedException e) {
                        throw new RuntimeException("panic: unexpected thread interruption");
                    }
                    tylkoJedenMozeSzukacCyklu.remove(componentId);
                    if (!tylkoJedenMozeSzukacCyklu.isEmpty()) { // Pozwalamy komus innemu szukac cyklu.
                        tylkoJedenMozeSzukacCyklu.values().iterator().next().release();
                    }
                    mutexCykl.release();

                    try {
                        mutex.acquire();
                    } catch (InterruptedException e) {
                        throw new RuntimeException("panic: unexpected thread interruption");
                    }

                    // Czekalismy na szukanie cyklu. W tym czasie
                    // moglo sie pojawic miejsce, wiec ponownie sprawdzamy.
                    if (!cel.czyWolneMiejsce() && !miejsceJuzZmniejszone.contains(transfer)) {
                        mutex.release();
                        try {
                            para.semafor.acquire();
                        } catch (InterruptedException e) {
                            throw new RuntimeException("panic: unexpected thread interruption");
                        }
                    } else {

                        if (!miejsceJuzZmniejszone.contains(transfer)) {
                            cel.liczbaWolnychMiejsc.decrementAndGet();
                            miejsceJuzZmniejszone.add(transfer);
                        }
                        cel.czekajaceNaMiejsce.remove(para);
                        mutex.release();
                    }
                }

                usunPolaczenieZGrafu(transfer);
            } else {
                cel.liczbaWolnychMiejsc.decrementAndGet();
                miejsceJuzZmniejszone.add(transfer);
                mutex.release();
            }

            // W tym miejscu jest cykl lub wolne miejsce.

            try {
                mutexCykl.acquire();
                mutex.acquire();
            } catch (InterruptedException e) {
                throw new RuntimeException("panic: unexpected thread interruption");
            }

            cel.czekajaceNaMiejsce.remove(para);
            mutex.release();

            if (cyklKopia.contains(transfer)) { // Transfer jest z cyklu.
                cyklKopia.remove(transfer);
                if (cyklKopia.isEmpty()) { // Jestesmy ostatnim wykonujacym sie transferem w cyklu.
                    cykl = null;
                    // Pozwalamy komus innemu szukac cyklu.
                    if (!tylkoJedenMozeSzukacCyklu.isEmpty()) {
                        // Zeby nie budzic samych siebie.
                        for (Map.Entry<ComponentId, Semaphore> entry : tylkoJedenMozeSzukacCyklu.entrySet()) {
                            if (!entry.getKey().equals(componentId)) {
                                entry.getValue().release();
                                break;
                            }
                        }
                    }
                }

                try {
                    mutex.acquire();
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }
                tylkoJedenMozeSzukacCyklu.remove(componentId);

                if (miejsceJuzZmniejszone.contains(transfer)) { // Oddaje miejsce z ktorego nie skorzystal.
                    if (!urzadzenia.get(transfer.getDestinationDeviceId()).czekajaceNaMiejsce.isEmpty()) {
                        urzadzenia.get(transfer.getDestinationDeviceId()).czekajaceNaMiejsce.get(0).semafor.release();
                    }

                    urzadzenia.get(transfer.getDestinationDeviceId()).liczbaWolnychMiejsc.incrementAndGet();
                    miejsceJuzZmniejszone.remove(transfer);
                }
                mutex.release();
                mutexCykl.release();

                zrodlo.zajeteMiejsca.remove(componentId);
                cel.zajeteMiejsca.add(componentId);
                transfer.prepare();

                czekanieNaPrepare.get(poprzednicy.get(transfer)).release(); // Po wykonaniu 'prepare' informujemy o tym naszego poprzednika.
                try {
                    czekanieNaPrepare.get(transfer).acquire(); // Sami czekamy na 'prepare' naszego nastepnika.
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }
                transfer.perform();
            }
            else { // Przeniesienie nie jest z cyklu.

                try {
                    mutex.acquire();
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }

                if (miejsceJuzZmniejszone.contains(transfer)) {
                    cel.zajeteMiejsca.add(componentId);
                    miejsceJuzZmniejszone.remove(transfer);
                } else {
                    cel.dodajKomponentDoPamieci(componentId);
                }
                zrodlo.usunKomponentZPamieci(componentId);
                transfer.prepare();

                // Usuwamy krawedz z grafu i budzimy transfer czekajacy na zwolnione miejsce.
                usunPolaczenieZGrafu(transfer);
                zrodlo.wznowPierwszyCzekajacyNieZCyklu(cykl, urzadzenia, miejsceJuzZmniejszone);
                transfer.perform();

                mutex.release();
                mutexCykl.release();
            }
        }

        try {
            mutexLiczenieAktywnychTransferow.acquire();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
        aktywneTransfery.remove(componentId);
        mutexLiczenieAktywnychTransferow.release();
    }
}
