import java.net.InetAddress;
import java.net.UnknownHostException;

public class DnsRequest {
    private String type;
    private String name;

    public DnsRequest(String type, String name) {
        if (type.equals("PTR") || type.equals("A")) {
            this.type = type;
            this.name = name;
        } else throw new IllegalArgumentException();
    }

    public String getResult() {
        StringBuilder result = new StringBuilder(String.format("%s:%s=", name, type));

        InetAddress host;
        try {
            host = InetAddress.getByName(name);
        } catch (UnknownHostException e) {
            return null;
        }
        if (type.equals("PTR")) {
            if (host.getHostName().equals(name)) //cant find hostname, getHostName() returns ip addr
                return null;
            result.append(host.getHostName());
        } else if (type.equals("A")) {
            if (isIpv4(name))
                return null;
            result.append(host.getHostAddress());
        }
        return result.toString();
    }

    public static boolean isIpv4(final String ip) {
        String PATTERN = "^((0|1\\d?\\d?|2[0-4]?\\d?|25[0-5]?|[3-9]\\d?)\\.){3}(0|1\\d?\\d?|2[0-4]?\\d?|25[0-5]?|[3-9]\\d?)$";

        return ip.matches(PATTERN);
    }
}
