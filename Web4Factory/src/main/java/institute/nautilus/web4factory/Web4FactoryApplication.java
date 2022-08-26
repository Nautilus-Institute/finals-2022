package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.integration.annotation.Router;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.context.annotation.Configuration;
import org.springframework.expression.Expression;
import org.springframework.expression.ExpressionParser;
import org.springframework.expression.spel.standard.SpelExpressionParser;
import org.springframework.integration.router.HeaderValueRouter;

import com.google.gson.*;
import com.google.gson.stream.*;

import java.security.Permission;
import java.util.Scanner;
import java.lang.*;
import java.util.*;
import java.io.*;


import institute.nautilus.web4factory.BoundedBufferedReader;
import institute.nautilus.web4factory.Web4Gateway;
import institute.nautilus.web4factory.Web4SecurityManager;
import institute.nautilus.web4factory.Web4Connection;
import institute.nautilus.web4factory.Web4FactoryParser;

@SpringBootApplication
public class Web4FactoryApplication {

    static Logger logger = LoggerFactory.getLogger(Web4FactoryApplication.class);
    Gson gson = new Gson();


      private static class ExitTrappedException extends SecurityException { }

      private static void forbidSystemExitCall() {
        final SecurityManager securityManager = new Web4SecurityManager();
        System.setSecurityManager( securityManager ) ;
        Properties props = System.getProperties();
        props.setProperty("jdk.xml.enableTemplatesImplDeserialization", "true");
      }

      /*
      private static void enableSystemExitCall() {
        System.setSecurityManager( null ) ;
      }
      */

    /*
    @Router(inputChannel = "web4RouterChannel")
	@Bean
	public Web4Router router() {
		Web4Router router = new Web4Router();
		return router;
	}

    @Bean
	public MessageChannel secretChannel() {
		DirectChannel channel = new DirectChannel();
		return channel;
	}

	@Bean
	public MessageChannel publicChannel() {
		DirectChannel channel = new DirectChannel();
		return channel;
	}

	@Bean
	public MessageChannel defaultOutputChannel() {
		DirectChannel channel = new DirectChannel();
		return channel;
	}

    @Autowired
	private Web4Gateway web4Gateway;
    */

    // java -cp ./target/web4factory.jar -Dloader.main=institute.nautilus.web4factory.LibFactororg.springframework.boot.loader.PropertiesLauncherer
	public static void main(String[] args) throws IOException {
        if (args.length > 0) {
            Web4FactoryParser.loadLibFactory();
            Web4FactoryParser.runParseLoop();
        } else {
            //System.in.readline();
            forbidSystemExitCall();
            Web4Connection.loadSodium();
            SpringApplication.run(Web4FactoryApplication.class, args);
        }
	}

    @Bean
	public CommandLineRunner demo() {
		return (args) -> {
            logger.info("Starting Web4");
            //System.out.println("");
            //

            /*

            Web4Connection conn = new Web4Connection(this);

            //Message<?> message = MessageBuilder.withPayload("Hello").setHeader("messageType", "secret").build();
            //web4Gateway.print(message);

            BufferedReader input_reader = new BoundedBufferedReader(
                    new InputStreamReader(System.in), 100, 10000);
            try {
                while (true) {
                    String line = input_reader.readLine();
                    conn.process_line(line);
                }
            } catch(Exception e) {
                e.printStackTrace();
            }
            */

            ///Message<?> message = MessageBuilder.withPayload("Hello").setHeader("actionName","foo").build();
            //web4Gateway.print(message);

            /*
			for (int i = 0; i < 10; i++) {
				Message<?> message = null;
				if (i % 2 == 0) {
					message = MessageBuilder.withPayload("secret message " + i).setHeader("messageType", "secret")
							.build();
				} else if (i % 3 == 0) {
					message = MessageBuilder.withPayload("public message " + i).setHeader("messageType", "public")
							.build();
				} else {
					message = MessageBuilder.withPayload("Generic message " + i).setHeader("messageType", "public")
							.build();
				}

				web4Gateway.print(message);
			}
            */

		};
	}


}
