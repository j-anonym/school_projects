import java.util.*;
import java.io.BufferedReader;
import java.io.IOException;

public class Request {
    private String method;
    private String path;
    private String fullUrl;
    private Integer contentLength = null;
    private Map<String, String> headers = new HashMap<String, String>();
    private Map<String, String> queryParameters = new HashMap<String, String>();
    private BufferedReader in;
    private String body = null;

    public Request(BufferedReader in) {
        this.in = in;
    }

    public String getMethod() {
        return method;
    }

    public String getPath() {
        return path;
    }

    public String getHeader(String headerName) {
        return headers.get(headerName);
    }

    public String getParameter(String paramName) {
        return queryParameters.get(paramName);
    }

    public Map<String, String> getParameters() {
        return queryParameters;
    }

    public Integer getContentLength() {
        return contentLength;
    }

    private void parseQueryParameters(String queryString) {
        for (String parameter : queryString.split("&")) {
            int separator = parameter.indexOf('=');
            if (separator > -1) {
                queryParameters.put(parameter.substring(0, separator),
                        parameter.substring(separator + 1));
            } else {
                queryParameters.put(parameter, null);
            }
        }
    }

    public boolean parse() throws IOException {
        String initialLine = in.readLine();
        if (initialLine == null) {

            return false;
        }
        System.out.println(initialLine);
        StringTokenizer tok = new StringTokenizer(initialLine);
        String[] components = new String[3];
        for (int i = 0; i < components.length; i++) {
            if (tok.hasMoreTokens()) {
                components[i] = tok.nextToken();
            } else {
                return false;
            }
        }

        method = components[0];
        fullUrl = components[1];

        // Consume headers
        while (true) {
            String headerLine = in.readLine();
            if (headerLine.length() == 0) {
                break;
            }

            int separator = headerLine.indexOf(":");
            if (separator == -1) {
                return false;
            }
            String header = headerLine.substring(0, separator);
            String header_value = headerLine.substring(separator + 1);
            headers.put(header.toLowerCase(), header_value);
        }

        if (method.equals("POST") && headers.containsKey("content-length")) {
            contentLength = Integer.valueOf(headers.get("content-length").trim());
            char[] body = new char[contentLength];
            if (in.read(body, 0, contentLength) != contentLength)
                return false;
            this.body = new String(body);
        }

        if (!components[1].contains("?")) {
            path = components[1];
        } else {
            path = components[1].substring(0, components[1].indexOf("?"));
            parseQueryParameters(components[1].substring(
                    components[1].indexOf("?") + 1));
        }

        return true;
    }

    public String getBody() {
        return body;
    }
}