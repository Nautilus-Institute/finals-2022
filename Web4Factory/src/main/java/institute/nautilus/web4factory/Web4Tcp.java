package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.annotation.IntegrationComponentScan;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.config.EnableIntegration;
import org.springframework.integration.ip.tcp.TcpInboundGateway;

import org.springframework.integration.ip.tcp.connection.AbstractServerConnectionFactory;
import org.springframework.integration.ip.tcp.connection.TcpNetServerConnectionFactory;
//import org.springframework.integration.ip.tcp.connection.Web4TcpNetServerConnectionFactory;
import org.springframework.integration.ip.tcp.serializer.ByteArrayLfSerializer;
import org.springframework.messaging.MessageChannel;

import institute.nautilus.web4factory.Web4Service;
import institute.nautilus.web4factory.Web4TcpInboundGateway;

@Configuration
@EnableIntegration
@IntegrationComponentScan
public class Web4Tcp {
    private static final Logger LOGGER = LoggerFactory.getLogger(Web4Tcp.class);
    
    private static final int TCP_SERVER_PORT = 10652;

    @Bean
    public AbstractServerConnectionFactory tcpServer() {
        LOGGER.info("Starting TCP server with port: {}", TCP_SERVER_PORT);
        TcpNetServerConnectionFactory serverCf = new TcpNetServerConnectionFactory(TCP_SERVER_PORT);
        //Web4TcpNetServerConnectionFactory serverCf = new Web4TcpNetServerConnectionFactory(TCP_SERVER_PORT);
        ByteArrayLfSerializer s = new ByteArrayLfSerializer();
        s.setMaxMessageSize(10000);
        serverCf.setSerializer(s);
        serverCf.setDeserializer(s);
        serverCf.setSoTcpNoDelay(true);
        serverCf.setSoKeepAlive(false);
        return serverCf;
    }

    @Bean
    public Web4Service tcpService() {
        return new Web4Service();
    }

    @Bean
    public MessageChannel fromTcp() {
        return new DirectChannel();
    }

    @Bean
    public MessageChannel toTcp() {
        return new DirectChannel();
    }
    @Bean
    public MessageChannel tcpError() {
        return new DirectChannel();
    }

    @Bean
    public TcpInboundGateway tcpInGate() {
        TcpInboundGateway inGate = new Web4TcpInboundGateway();
        inGate.setConnectionFactory(tcpServer());
        inGate.setRequestChannel(fromTcp());
        inGate.setRequestTimeout(45000);
        inGate.setReplyChannel(toTcp());
        inGate.setErrorChannel(tcpError());
        return inGate;
    }
}

