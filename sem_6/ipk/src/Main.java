import static java.lang.System.exit;

public class Main {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("ERROR: Wrong arguments, should run only with one argument <port_number>\nin range <1024,65353>");
            exit(1);
        }
        int port = -1;

        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.err.println("ERROR: Wrong arguments, should run only with one argument <port_number>\nin range <1024,65353>");
            exit(1);
        }
        if (port < 0) {
            System.err.println("ERROR: Wrong arguments, port has to be positive number\nin range <1024,65353>");
            exit(1);
        } else if (port < 1024) {
            System.err.println("ERROR: Wrong arguments, reserved port\nport has to be in range <1024,65353>");
            exit(1);
        } else if (port > 65353) {
            System.err.println("ERROR: Wrong arguments\nport has to be in range <1024,65353>");
            exit(1);
        }
        Server server = new Server(port);
        server.run();
    }

}
