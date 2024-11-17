import java.rmi.RemoteException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

class GFGImplementation implements gaussianElimination {
    public static int N;

    //SEQUENTIALLY
    @Override
    public double[] gaussianEliminationSeq(double mat[][], int N) {
        this.N = N;
        // sprowadzenie macierzy do postaci schodkowej
        int singular_flag = forwardElim(mat);

        // Sprawdzamy czy macierz jest osobliwa
        if (singular_flag != -1) {
            System.out.println("Singular Matrix.");

            // sprawdzamy czy wartosc po prawej stronie równania jest równa
            // jeżeli tak -> nieskończenie wiele rozwiązań
            // jeżeli nie -> brak rozwiązań
            if (mat[singular_flag][N] != 0)
                System.out.print("Inconsistent System.");
            else
                System.out.print(
                        "May have infinitely many solutions.");

            return null;
        }
 
    /* get solution to system and print it using
           backward substitution */
        return backSub(mat);
    }

    // funkcja do zamieniania dwóch rzędów miejscami
    private static void swap_row(double mat[][], int i, int j) {
        // printf("Swapped rows %d and %d\n", i, j);

        for (int k = 0; k <= N; k++) {
            double temp = mat[i][k];
            mat[i][k] = mat[j][k];
            mat[j][k] = temp;
        }
    }

    // function to print matrix content at any stage
    private static void print(double mat[][]) {
        for (int i = 0; i < N; i++, System.out.println())
            for (int j = 0; j <= N; j++)
                System.out.print(mat[i][j]);
        System.out.println();
    }

    // tworzenie macierzy schodkowej
    private static int forwardElim(double mat[][]) {
        for (int k = 0; k < N; k++) {
            // k -> który schodek robimy

            // ustawiamy wartości pivot'ów na 0,0 - posłużą nam one do znalezienia największego elementu
            int i_max = k;
            int v_max = (int) mat[i_max][k];

            // szukanie największej wartosci w KOLUMNIE
            for (int i = k + 1; i < N; i++)
                if (Math.abs(mat[i][k]) > v_max) {
                    v_max = (int) mat[i][k];
                    i_max = i;
                }

            // sprawdzamy przed zamianą czy wartosc ktora znajdzie sie na przekatnej glownej nie bedzie zerowa
            // jezeli jest zerowa oznacza to ze macierz jest osobliwa
            if (mat[k][i_max] == 0)
                return k;

            // zamiana rzędu z największą pierwszą wartoscią z aktualnym rzędem (algorytm tego pragnie)
            // https://e.kul.pl/files/10382/public/aan_w6_1819_1.pdf
            if (i_max != k)
                swap_row(mat, k, i_max);


            for (int i = k + 1; i < N; i++) {

                // obliczamy F przez ktory bedziemy zmeiniac nastepny schodek
                double f = mat[i][k] / mat[k][k];

                // obliczanie wspolczynnikow w nastepnym schodku
                for (int j = k + 1; j <= N; j++)
                    mat[i][j] -= mat[k][j] * f;

                // zmieniamy pierwsza wartosc nastepnego schodka na 0
                mat[i][k] = 0;
            }

            // print(mat);
        }

        // print(mat);
        return -1;
    }

    // funkcja obliczajaca wartosci niewiadomych
    private static double[] backSub(double mat[][]) {
        double x[] = new double[N]; // tablica przechowujaca wyniki

        // zaczynamy od konca
        for (int i = N - 1; i >= 0; i--) {

            // przypisujemy do tablicy wynikowa prawa strone macierzy (wartosci po prawej stronie rownania)
            x[i] = mat[i][N];

            for (int j = i + 1; j < N; j++) {

                // odejmujemy od wartosci po prawej stronie rownania wszystkie wspolczynniki ktore juz znamy
                x[i] -= mat[i][j] * x[j];
            }

            // dzielimy prawa strone przez to co stoi przy naszym x zeby obliczyc wartosc niewiadomej.
            x[i] = x[i] / mat[i][i];
        }

        System.out.println();
        System.out.println("Solution for the system:");
        for (int i = 0; i < N; i++) {
            System.out.print("x" + i + ". ");
            System.out.format("%.6f", x[i]);
            System.out.println();
        }
        return x;
    }

    //CONCURRENTLY
    @Override
    public double[] gaussianEliminationThreaded(double mat[][], int N){
        this.N = N;
        // sprowadzenie macierzy do postaci schodkowej
        int singular_flag = forwardElimThreaded(mat);

        // Sprawdzamy czy macierz jest osobliwa
        if (singular_flag != -1) {
            System.out.println("Singular Matrix.");

            // sprawdzamy czy wartosc po prawej stronie równania jest równa
            // jeżeli tak -> nieskończenie wiele rozwiązań
            // jeżeli nie -> brak rozwiązań
            if (mat[singular_flag][N] != 0)
                System.out.print("Inconsistent System.");
            else
                System.out.print(
                        "May have infinitely many solutions.");

            return null;
        }

    /* get solution to system and print it using
           backward substitution */
        return backSub(mat);
    }

    private static int forwardElimThreaded(double mat[][]) {
        ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());

        for (int k = 0; k < N; k++) {
            // Pivotowanie
            int i_max = k;
            double v_max = Math.abs(mat[i_max][k]);

            // Znalezienie największego elementu w kolumnie poniżej przekątnej
            for (int i = k + 1; i < N; i++) {
                if (Math.abs(mat[i][k]) > v_max) {
                    v_max = Math.abs(mat[i][k]);
                    i_max = i;
                }
            }

            // Sprawdzenie osobliwości macierzy
            if (v_max == 0) {
                executor.shutdownNow();
                return k; // Macierz osobliwa
            }

            // Zamiana wierszy (pivot)
            if (i_max != k) {
                swap_row(mat, k, i_max);
            }

            // Ustalony pivot, eliminujemy elementy poniżej
            final int pivotRow = k;

            // Użycie ExecutorService dla równoległego przetwarzania wierszy
            for (int i = k + 1; i < N; i++) {
                final int currentRow = i;
                executor.submit(() -> {
                    double factor = mat[currentRow][pivotRow] / mat[pivotRow][pivotRow];
                    for (int j = pivotRow + 1; j <= N; j++) {
                        synchronized (mat) { // Synchronizacja dostępu do wspólnej macierzy
                            mat[currentRow][j] -= mat[pivotRow][j] * factor;
                        }
                    }
                    mat[currentRow][pivotRow] = 0; // Zerowanie wartości pod pivotem
                });
            }

            // Synchronizacja po każdej iteracji pivotu
            executor.shutdown();
            try {
                executor.awaitTermination(Long.MAX_VALUE, java.util.concurrent.TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            // Restart ExecutorService dla następnej iteracji
            executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
        }

        executor.shutdown();
        try {
            executor.awaitTermination(Long.MAX_VALUE, java.util.concurrent.TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        return -1; // Brak błędu
    }


}