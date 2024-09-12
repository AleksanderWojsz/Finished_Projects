package cp2023.solution;

import java.util.HashMap;
import java.util.Map;
import cp2023.base.*;

public final class StorageSystemFactory {

    public static StorageSystem newSystem(
            Map<DeviceId, Integer> deviceTotalSlots,
            Map<ComponentId, DeviceId> componentPlacement) {

        HashMap<DeviceId, Urzadzenie> urzadzenia = new HashMap<>();

        Wyjatki.czyBrakUrzadzen(deviceTotalSlots);

        // Tworzenie nowych urzadzen i dodawanie ich do mapy.
        for (DeviceId deviceId : deviceTotalSlots.keySet()) {
            Wyjatki.czyUrzadzenieNieJestNullem(deviceId);
            Wyjatki.czyPojemnoscUrzadzeniaPoprawna(deviceId, deviceTotalSlots);
            urzadzenia.put(deviceId, new Urzadzenie(deviceId, deviceTotalSlots.get(deviceId)));
        }

        // Dodawanie startowych komponentow do urzadzen.
        for (Map.Entry<ComponentId, DeviceId> entry : componentPlacement.entrySet()) {
            Wyjatki.czyKomponentNieJestNullem(entry.getKey());
            Wyjatki.czyWskazaneUrzadzenieIstniejeWSystemie(entry, deviceTotalSlots);
            urzadzenia.get(entry.getValue()).dodajKomponentDoPamieci(entry.getKey());
            Wyjatki.czyPrzekoroczonoLimitPamieci(urzadzenia, entry);
        }

        return new Storage(urzadzenia);
    }
}
