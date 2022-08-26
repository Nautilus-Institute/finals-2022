package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.springframework.integration.ip.tcp.TcpInboundGateway;
import org.springframework.integration.ip.tcp.connection.TcpConnection;

public class Web4TcpInboundGateway extends TcpInboundGateway {
    private static final Logger logger = LoggerFactory.getLogger(Web4Tcp.class);

    @Override
    public void removeDeadConnection(TcpConnection connection) {
        //logger.info("TCP connection exited {}", connection);

        Web4Service.closeSession(connection.getConnectionId());

        super.removeDeadConnection(connection);
    }
}
