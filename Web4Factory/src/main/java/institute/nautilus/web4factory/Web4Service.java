package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.integration.router.HeaderValueRouter;
import org.springframework.integration.annotation.MessageEndpoint;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.context.annotation.Bean;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHeaders;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.stereotype.Component;

import java.util.Scanner;
import java.lang.*;
import java.util.*;
import java.io.*;

import java.nio.charset.StandardCharsets;

import institute.nautilus.web4factory.Web4Connection;
//import institute.nautilus.web4factory.LibFactory;
import org.springframework.context.annotation.Configuration;

@Component
@MessageEndpoint
@Configuration
public class Web4Service {
    private static final Logger logger = LoggerFactory.getLogger(Web4Service.class);

    static final Map<String,Web4Connection> sessions = Collections.synchronizedMap(new HashMap());

    @ServiceActivator(inputChannel = "web4ActionChannel")
    @Bean
    public HeaderValueRouter actionRouter() {
        HeaderValueRouter router = new HeaderValueRouter("actionName");
        router.setResolutionRequired(false);
        //router.setChannelMapping("foo", "secretChannel");
        router.setDefaultOutputChannelName("unkownAction");
        return router;
    }

    @ServiceActivator(inputChannel = "web4AccessChannel")
    @Bean
    public HeaderValueRouter accessRouter() {
        HeaderValueRouter router = new HeaderValueRouter("path");
        router.setResolutionRequired(false);
        router.setDefaultOutputChannelName("invalidAccess");
        return router;
    }

    @Autowired
    private Web4Gateway web4Gateway;

    @ServiceActivator(inputChannel = "tcpError")
    public void handleError(Message<?> message) {
        MessageHeaders headers = message.getHeaders();
        String connid = (String)headers.get("ip_connectionId");
        //logger.info("TCP Error! Connid = {} Sessions: {}", connid, this.sessions);

        this.closeSession(connid);
    }

    static void closeSession(String connid) {
        Web4Connection conn = sessions.get(connid);
        if (conn == null) {
            //logger.info("{} closed session before connection started", connid);
            return;
        }
        conn.libfactory_destroy();
        sessions.remove(connid);
    }

    @ServiceActivator(inputChannel = "fromTcp")
    public void handleMessage(Message<byte[]> message) {
        try {
            //logger.info("Got message fromTcp: {} using {} (pid {})", message, this, 0);
            byte[] msg = message.getPayload();

            String in_message = new String(msg);
            //logger.info("Received request fromTcp: {}", in_message);

            // [payload=byte[6], headers={replyChannel=org.springframework.messaging.core.GenericMessagingTemplate$TemporaryReplyChannel@6c7e6c1a, errorChannel=org.springframework.messaging.core.GenericMessagingTemplate$TemporaryReplyChannel@6c7e6c1a, ip_tcp_remotePort=50820, ip_connectionId=localhost:50820:1111:04681504-7243-4d23-89fd-2ae419de3ce0, ip_localInetAddress=/127.0.0.1, ip_address=127.0.0.1, id=33e431bb-29a2-de80-c19e-57f75764d2ea, ip_hostname=localhost, timestamp=1658280734303}]
            MessageHeaders headers = message.getHeaders();

            //Message<?> message_out = MessageBuilder.withPayload("hello_from_server".getBytes(StandardCharsets.UTF_8)).build();

            String connid = (String)headers.get("ip_connectionId");
            //logger.info("Connid = {} Sessions: {}", connid, this.sessions);

            Web4Connection conn = this.sessions.get(connid);
            if (conn == null) {
                //logger.info("Creating new session for `{}`", connid);
                conn = new Web4Connection();
                this.sessions.put(connid, conn);
                //logger.info("Connid = {} Sessions: {}", connid, this.sessions);
            } else {
                conn = this.sessions.get(connid);
            }
            MessageChannel reply_chan = (MessageChannel)headers.getReplyChannel();
            //logger.info("Reply channel {}", reply_chan);
            conn.setReplyChannel(reply_chan);
            Web4Action action = conn.process_line(in_message);
            if (action != null) {
                Message<Web4Action> actionMsg = MessageBuilder.withPayload(action)
                    .setHeader("actionName", action.name)
                    .setHeader("service", this)
                    .setHeader("connection", conn)
                    .build();
                web4Gateway.handleAction(actionMsg);
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public void accessRoute(Web4Connection conn, String path) {
        String fullpath = conn.host_path + path;
        Object data = conn.getAccessData();
        if (data == null) {
            data = "";
        }
        //logger.info("Launching route `{}` {}", fullpath, data);
        Message<Object> accessMsg = MessageBuilder.withPayload(data)
            .setHeader("path", fullpath)
            .setHeader("service", this)
            .setHeader("connection", conn)
            .build();
        web4Gateway.handleAccess(accessMsg);
    }
    /*
    public byte[] handleMessage(byte[] msg) {
        return "hello_from_server".getBytes(StandardCharsets.UTF_8);
    }
    */
}
