import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.Random;


public class Client {

    public static double[][] generateMatrix(int size) {
        Random rand = new Random();
        double[][] matrix = new double[size][size + 1];

        for (int i = 0; i < size; i++) {
            for (int j = 0; j <= size; j++) {
                matrix[i][j] = rand.nextDouble() * 200 - 100;
            }
        }

        return matrix;
    }

    public static void printResults(double resultsArray[], int size){

        if(resultsArray == null){
            System.out.println("No solution found");
        } else {
            for (int i = 0; i < size; i++) {
                System.out.print("x" + i + ". ");
                System.out.format("%.6f", resultsArray[i]);
                System.out.println();
            }
        }
    }

    public static void main(String[] args) {
        try {
            Registry registry = LocateRegistry.getRegistry("localhost", 1099);
            gaussianElimination calc = (gaussianElimination) registry.lookup("gaussianElimination");

            long start, finish, timeElapsed;
            int size = 4000;

            double mat[][] = generateMatrix(4000);
            double resultsArray[];

            start = System.currentTimeMillis();
            resultsArray = calc.gaussianEliminationSeq(mat, size);
            finish = System.currentTimeMillis();
            timeElapsed = finish - start;

            System.out.println();
            System.out.println("Solution for the system and sample size " + size + " (ELAPSED TIME: "
                    + timeElapsed + "ms.): ");
            //printResults(resultsArray, size);

            start = System.currentTimeMillis();
            resultsArray = calc.gaussianEliminationThreaded(mat, size);
            finish = System.currentTimeMillis();
            timeElapsed = finish - start;

            System.out.println();
            System.out.println("Threaded Solution for the system and sample size " + size + " (ELAPSED TIME: "
                    + timeElapsed + "ms.): ");
            //printResults(resultsArray, size);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}