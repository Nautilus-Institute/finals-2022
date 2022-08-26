package institute.nautilus.web4factory;

import java.lang.*;
import java.util.*;
import java.util.jar.*;
import java.io.*;
import java.net.*;
import java.nio.*;

import org.springframework.integration.annotation.Gateway;
import org.springframework.integration.annotation.MessagingGateway;
import org.springframework.messaging.Message;

import institute.nautilus.web4factory.Web4Connection;
import institute.nautilus.web4factory.Web4FactoryParser;

@MessagingGateway(name = "web4Gateway", defaultRequestChannel="web4ActionChannel")
public interface Web4Gateway {

	@Gateway(requestChannel = "web4ActionChannel")
	public void handleAction(Message<?> message);

	@Gateway(requestChannel = "web4AccessChannel")
	public void handleAccess(Message<?> message);

    public static void main(String[] args) throws IOException {
        Web4FactoryParser.loadLibFactory();
        Web4FactoryParser.runParseLoop();
    }
}
