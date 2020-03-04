import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

class Handler implements Runnable {
    private Socket socket;

    public Handler(Socket socket) {
        this.socket = socket;
    }

    private void respond(int statusCode, String msg, OutputStream out) throws IOException {
        String responseLine = "HTTP/1.1 " + statusCode + " " + msg + "\r\n";
        out.write(responseLine.getBytes());
    }

    public void run() {
        BufferedReader in = null;
        OutputStream out = null;

        try {
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = socket.getOutputStream();

            Request request = new Request(in);
            if (!request.parse()) {
                respond(500, "Can't parse request", out);
                return;
            }

            Response response = new Response(out);

            if (request.getMethod().equals("GET")) {
                processGet(request, response);
            } else if (request.getMethod().equals("POST")) {
                processPost(request, response);
            } else {
                respond(405, "Method Not Allowed", out);
                return;
            }


        } catch (IOException e) {
            try {
                e.printStackTrace();
                if (out != null) {
                    respond(500, e.toString(), out);
                }
            } catch (IOException e2) {
                e2.printStackTrace();
                // We tried
            }
        } finally {
            try {
                if (out != null) {
                    out.flush();
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void processPost(Request request, Response response) throws IOException {
        boolean hasName = false, hasType = false;
        String type = null, name = null;

        if (!request.getPath().equals("/dns-query")) {
            respond(400, "Bad Request", socket.getOutputStream());
            return;
        }

        if (request.getBody() == null) {
            respond(400, "Bad Request", socket.getOutputStream());
            return;
        }

        try {
            request.getBody()
                    .lines()
                    .map(this::processBodyLine)
                    .map(DnsRequest::getResult)
                    .filter(Objects::nonNull)
                    .forEach(response::addBody);

        } catch (RuntimeException e) {
            respond(400, "Bad Request", socket.getOutputStream());
            return;
        }

        response.setResponseCode(200, "OK");
        response.addHeader("Content-Type", "text/plain");
        response.send();


    }

    private DnsRequest processBodyLine(String line) throws RuntimeException {
        if (!line.contains(":")) {
            throw new RuntimeException("Wrong format");
        }
        String[] parts = line.split(":", 2);
        return new DnsRequest(parts[1], parts[0]);
    }

    private void processGet(Request request, Response response) throws IOException {
        boolean hasName = false, hasType = false;
        String type = null, name = null;

        if (!request.getPath().equals("/resolve")) {
            respond(400, "Bad Request", socket.getOutputStream());
            return;
        }
        for (var param : request.getParameters().entrySet()) {
            if (param.getKey().equals("name")) {
                hasName = true;
                name = param.getValue();
            } else if (param.getKey().equals("type")) {
                hasType = true;
                type = param.getValue();
            } else {
                respond(400, "Bad Request", socket.getOutputStream());
                return;
            }
        }
        if (!(hasName && hasType)) {
            respond(400, "Bad Request", socket.getOutputStream());
            return;
        }

        String result;
        try {
            DnsRequest dnsRequest = new DnsRequest(type, name);
            result = dnsRequest.getResult();
            if (result == null) {
                respond(404, "Not Found", socket.getOutputStream());
                return;
            }
        } catch (IllegalArgumentException ex) {
            respond(400, "Request", socket.getOutputStream());
            return;
        }

        response.setResponseCode(200, "OK");
        response.addBody(result);
        response.addHeader("Content-Type", "text/plain");
        response.send();

    }
}