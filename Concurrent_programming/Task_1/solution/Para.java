package cp2023.solution;

import cp2023.base.ComponentTransfer;
import java.util.concurrent.Semaphore;

public class Para {

    protected ComponentTransfer transfer;
    protected Semaphore semafor;

    public Para(ComponentTransfer transfer) {
        this.transfer = transfer;
        this.semafor = new Semaphore(0);
    }
}
