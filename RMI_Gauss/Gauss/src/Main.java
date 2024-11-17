import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;


public class Main {

    public static void main(String[] args) throws RemoteException {

        gaussianElimination server = new GFGImplementation();

        gaussianElimination stub = (gaussianElimination) UnicastRemoteObject
                .exportObject((gaussianElimination) server, 0);

        Registry registry = LocateRegistry.createRegistry(1099);

        registry.rebind("gaussianElimination", stub);

    }
}