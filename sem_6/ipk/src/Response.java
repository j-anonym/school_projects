import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

/**
 * Class that encapsulates HTTP response
 */
public class Response  {
    private OutputStream out;
    private int statusCode;
    private String statusMessage;
    private Map<String, String> headers = new HashMap<>();
    private int contentLength = 0;
    private List<String> body = new ArrayList<>();

    public Response(OutputStream out)  {
        this.out = out;
    }

    public void setResponseCode(int statusCode, String statusMessage)  {
        this.statusCode = statusCode;
        this.statusMessage = statusMessage;
    }

    public void addHeader(String headerName, String headerValue)  {
        this.headers.put(headerName, headerValue);
    }

    public void addBody(String body)  {
        this.body.add(body + "\n");
        contentLength += body.length() + 1;
    }

    public void send() throws IOException  {
        headers.put("Content-Length", Integer.toString(contentLength));
        headers.put("Connection", "Close");
        out.write(("HTTP/1.1 " + statusCode + " " + statusMessage + "\r\n").getBytes());
        for (String headerName : headers.keySet())  {
            out.write((headerName + ": " + headers.get(headerName) + "\r\n").getBytes());
        }
        out.write("\r\n".getBytes());
        for (String bodyPart: body) {
            out.write(bodyPart.getBytes());
        }
    }
}