import java.rmi.Remote;
import java.rmi.RemoteException;

public interface gaussianElimination extends Remote {
    double[] gaussianEliminationSeq(double mat[][], int N) throws RemoteException;
    double[] gaussianEliminationThreaded(double mat[][], int N) throws RemoteException;
}
