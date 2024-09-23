package cp2023.solution;

import cp2023.base.*;
import cp2023.exceptions.*;
import java.util.*;
import java.util.concurrent.Semaphore;

public class Wyjatki {

    public static void czyParametryTransferuPoprawne(ComponentTransfer transfer) throws IllegalTransferType {
        if (transfer.getSourceDeviceId() == null && transfer.getDestinationDeviceId() == null) {
            throw new IllegalTransferType(transfer.getComponentId());
        }
    }

    public static void czyCelIZrodloIstnieja(ComponentTransfer transfer, Map<DeviceId, Urzadzenie> urzadzenia) throws DeviceDoesNotExist {

        if (transfer.getSourceDeviceId() != null && !urzadzenia.containsKey(transfer.getSourceDeviceId())) {
            throw new DeviceDoesNotExist(transfer.getSourceDeviceId());
        }

        if (transfer.getDestinationDeviceId() != null && !urzadzenia.containsKey(transfer.getDestinationDeviceId())) {
            throw new DeviceDoesNotExist(transfer.getDestinationDeviceId());
        }
    }

    public static void czyDodawanyKomponentJuzWSystmie(ComponentId id, Map<DeviceId, Urzadzenie> urzadzenia) throws ComponentAlreadyExists {

        for (Urzadzenie urzadzenie : urzadzenia.values()) {
            if (urzadzenie.zajeteMiejsca.contains(id)) {
                throw new ComponentAlreadyExists(id, urzadzenie.idUrzadzenia);
            }
        }
    }

    public static void czyKomponentNaWskazanymUrzadzeniu(ComponentId id, Urzadzenie urzadzenie) throws ComponentDoesNotExist {
        if (!urzadzenie.zajeteMiejsca.contains(id)) {
            throw new ComponentDoesNotExist(id, urzadzenie.idUrzadzenia);
        }
    }

    public static void czyKomponentPotrzebujeTransferu(ComponentTransfer transfer) throws ComponentDoesNotNeedTransfer {
        if (transfer.getSourceDeviceId().equals(transfer.getDestinationDeviceId())) {
            throw new ComponentDoesNotNeedTransfer(transfer.getComponentId(), transfer.getSourceDeviceId());
        }
    }

    public static void czyKomponentJeszczeTransferowany(ComponentId componentId,Set<ComponentId> aktywneTransfery, Semaphore liczenieAktywnychTransferow) throws ComponentIsBeingOperatedOn {
        try {
            if (aktywneTransfery.contains(componentId)) {
                throw new ComponentIsBeingOperatedOn(componentId);
            }
            aktywneTransfery.add(componentId);
        } finally {
            liczenieAktywnychTransferow.release();
        }
    }

    public static void czyPojemnoscUrzadzeniaPoprawna(DeviceId deviceId, Map<DeviceId, Integer> deviceTotalSlots) throws IllegalArgumentException {
        Integer pojemnosc = deviceTotalSlots.get(deviceId);

        if (pojemnosc == null || pojemnosc <= 0) {
            throw new IllegalArgumentException("Device " + deviceId + " has invalid number of slots: " + pojemnosc);
        }
    }

    public static void czyPrzekoroczonoLimitPamieci(HashMap<DeviceId, Urzadzenie> urzadzenia, Map.Entry<ComponentId, DeviceId> entry) {
        if (urzadzenia.get(entry.getValue()).zajeteMiejsca.size() > urzadzenia.get(entry.getValue()).maksMiejsc) {
            throw new IllegalArgumentException("Number of components exceeds " +  entry.getValue() + "'s capacity of " + urzadzenia.get(entry.getValue()).maksMiejsc);
        }
    }

    public static void czyBrakUrzadzen(Map<DeviceId, Integer> deviceTotalSlots) {
        if (deviceTotalSlots.isEmpty()) {
            throw new IllegalArgumentException("No devices specified");
        }
    }

    public static void czyKomponentNieJestNullem(ComponentId componentId) {
        if (componentId == null) {
            throw new IllegalArgumentException("Component is null");
        }
    }

    public static void czyUrzadzenieNieJestNullem(DeviceId deviceId) {
        if (deviceId == null) {
            throw new IllegalArgumentException("Device is null");
        }
    }

    public static void czyWskazaneUrzadzenieIstniejeWSystemie(Map.Entry<ComponentId, DeviceId> entry, Map<DeviceId, Integer> deviceTotalSlots) {
        if (entry.getValue() == null || !deviceTotalSlots.containsKey(entry.getValue())) {
            throw new IllegalArgumentException("Component " + entry.getKey() + " assigned to a non-existent device: " + entry.getValue());
        }
    }
}
