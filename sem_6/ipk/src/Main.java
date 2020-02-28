import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Date;

public class Main {
    public static void main(String[] args) throws Exception {
        //TODO process arguments
        int port = 5353;
        Server server = new Server(port);
        server.run();
    }

}
