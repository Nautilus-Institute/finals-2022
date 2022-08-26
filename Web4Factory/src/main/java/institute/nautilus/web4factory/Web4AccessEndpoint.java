package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.security.MessageDigest;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.charset.StandardCharsets;
import org.apache.commons.codec.binary.Hex;
import org.apache.commons.lang.StringUtils;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Component;

import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.Web4Action;
import institute.nautilus.web4factory.Web4Service;
import institute.nautilus.web4factory.Web4Connection;
import institute.nautilus.web4factory.Web4FactoryException;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactory;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

@Component
public class Web4AccessEndpoint {
    static Logger logger = LoggerFactory.getLogger(Web4Connection.class);

	@ServiceActivator(inputChannel = "invalidAccess")
    public void invalidRoute(Message<?> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");
        conn.send_fault(String.format("Unkown route `%s`", message.getHeaders().get("path")));
    }

	@ServiceActivator(inputChannel = "/")
    public void indexRoute(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");

        Object data = message.getPayload();
        //logger.info("Index called with {}", data); 

        conn.send_data("web4factory portal access ok");
    }

	@ServiceActivator(inputChannel = "/serial")
    public void serial(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");

        Object data = message.getPayload();

        String hdata = "";
        if (data != null) {
            if (data instanceof String) {
                hdata += (String)data;
            } else {
                conn.send_fault("Expected string");
                return;

            }
        }

        String flag;
        try {
            flag = new String(Files.readAllBytes(Paths.get("./flag")));
        } catch(IOException e) {
            conn.send_fault("Failed to get serial file");
            return;
        }
        hdata += flag.trim();

        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            //logger.info("Taking sha256({})", hdata);
            byte[] encodedhash = digest.digest(
              hdata.getBytes(StandardCharsets.UTF_8));
            String res = Hex.encodeHexString(encodedhash);
            conn.send_data(res);
        } catch(java.security.NoSuchAlgorithmException e) {
            conn.send_fault("Failed to calculate serial");
            return;
        }
    }

	@ServiceActivator(inputChannel = "/factory/save")
    public void factorySaveRoute(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");
        if (conn.factory == null) {
            conn.send_fault("No active factory");
            return;
        }
        //conn.send_data("Factory configuration locked, contact administrator");


        String data = conn.factory.toString();

        if (data.length() == 0) {
            conn.send_fault(String.format("Failed to save factory `%s`", conn.factory.factoryName));
            return;
        }
        conn.send_data(data);
        //System.exit(0);
    }

	@ServiceActivator(inputChannel = "/factory/describe")
    public void factoryDescription(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");
        if (conn.factory == null) {
            conn.send_fault("No active factory");
            return;
        }
        conn.send_data(conn.factory.factoryExpr.expr);
    }


	@ServiceActivator(inputChannel = "/factory/feed")
    public void factoryFeedRoute(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");
        if (conn.factory == null) {
            conn.send_fault("No active factory");
            return;
        }

        Object input = message.getPayload();
        if (input == null) {
            conn.send_fault("Expected non null data");
            return;
        }

        try {
            Object result = conn.factory.feed(input);
            conn.send_data(result);
        } catch (Exception e) {
            e.printStackTrace();
            conn.send_fault("Error feeding factory");
        }
    }

	@ServiceActivator(inputChannel = "/factory/load/existing")
    public void factoryLoadRoute(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");

        //Web4FactoryFactory f = new Web4FactoryFactory("test", new Web4FactoryFactoryWidget("Hello world"));
        Object access_data = message.getPayload();
        if (!(access_data instanceof String)) {
            conn.send_fault("Expected factory string");
            return;
        }

        Web4FactoryFactory f = conn.loadFactory((String)access_data);
        if (f == null) {
            conn.send_fault("Failed to load factory");
            return;
        }
        conn.send_data(String.format("factory `%s` load ok", f.factoryName));

        /*
        Object res = expr.exec(null);
        logger.info("Result of expression is {}", res);
        */
    }

    @ServiceActivator(inputChannel = "/factory/load/config")
    public void testRoute(Message<Object> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");
        Object access_data = message.getPayload();

        Web4FactoryFactory f = conn.libfactory_parse(access_data);
        //logger.info("Result of parse {}", f);
        if (f == null) {
            conn.send_fault("Failed to parse data");
            return;
        }
        conn.send_data(String.format("factory `%s` successfully created from config", f.factoryName));
    }
}
