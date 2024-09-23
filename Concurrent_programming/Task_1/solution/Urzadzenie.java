package cp2023.solution;

import cp2023.base.*;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Urzadzenie {

    protected DeviceId idUrzadzenia;
    protected int maksMiejsc;
    protected List<ComponentTransfer> listaNiewykonanychTransferowWyjsciowych;
    protected List<Para> czekajaceNaMiejsce;
    protected List<ComponentId> zajeteMiejsca;
    protected AtomicInteger liczbaWolnychMiejsc;

    public Urzadzenie(DeviceId idUrzadzenia, int maksMiejsc) {
        this.idUrzadzenia = idUrzadzenia;
        this.maksMiejsc = maksMiejsc;
        this.listaNiewykonanychTransferowWyjsciowych = Collections.synchronizedList(new ArrayList<>());
        this.czekajaceNaMiejsce = Collections.synchronizedList(new ArrayList<>());
        this.zajeteMiejsca = Collections.synchronizedList(new ArrayList<>());
        this.liczbaWolnychMiejsc = new AtomicInteger(maksMiejsc);
    }

    void dodajKomponentDoPamieci(ComponentId componentId) {
        liczbaWolnychMiejsc.decrementAndGet();
        zajeteMiejsca.add(componentId);
    }

    void usunKomponentZPamieci(ComponentId componentId) {
        zajeteMiejsca.remove(componentId);
        liczbaWolnychMiejsc.incrementAndGet();
    }

    void wznowPierwszyCzekajacy() {
        if (!czekajaceNaMiejsce.isEmpty()) {
            Para para = czekajaceNaMiejsce.get(0);
            czekajaceNaMiejsce.remove(0);
            para.semafor.release();
        }
    }

    void wznowPierwszyCzekajacyNieZCyklu(List<ComponentTransfer> cykl, HashMap<DeviceId, Urzadzenie> urzadzenia, Set<ComponentTransfer> miejsceJuzZmniejszone) {

        if (!czekajaceNaMiejsce.isEmpty()) {
            for (int i = 0; i < czekajaceNaMiejsce.size(); i++) {
                Para para = czekajaceNaMiejsce.get(i);
                if (cykl == null || !cykl.contains(para.transfer)) {
                    czekajaceNaMiejsce.remove(i);

                    urzadzenia.get(para.transfer.getDestinationDeviceId()).liczbaWolnychMiejsc.decrementAndGet();
                    miejsceJuzZmniejszone.add(para.transfer);

                    if (para.transfer.getSourceDeviceId() != null && para.transfer.getDestinationDeviceId() != null) {
                        urzadzenia.get(para.transfer.getSourceDeviceId()).listaNiewykonanychTransferowWyjsciowych.remove(para.transfer);
                    }

                    para.semafor.release();
                    break;
                }
            }
        }
    }

    // Do budzenia transferow z cyklu.
    void wznowWskazanyCzekajacy(ComponentTransfer componentTransfer) {
        for (int i = 0; i < czekajaceNaMiejsce.size(); i++) {
            Para para = czekajaceNaMiejsce.get(i);
            if (para.transfer.equals(componentTransfer)) {
                czekajaceNaMiejsce.remove(para);
                para.semafor.release();
                break;
            }
        }
    }

    boolean czyWolneMiejsce() {
        return liczbaWolnychMiejsc.get() > 0;
    }
}
