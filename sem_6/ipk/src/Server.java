import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;


public class Server {
    private int port;
    private ServerSocket serverSocket;

    public Server(int port) {
        this.port = port;
    }

    public void run() {
        try {
            this.serverSocket = new ServerSocket(port);
        } catch (IOException e) {
            System.err.println("Cant create socket for server");
        }
        Socket client;
        try {
            while ((client = serverSocket.accept()) != null) {

                Handler handler = new Handler(client);
                Thread t = new Thread(handler);
                t.start();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
