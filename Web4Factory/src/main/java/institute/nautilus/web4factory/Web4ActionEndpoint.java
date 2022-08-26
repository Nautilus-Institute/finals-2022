package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Component;

import institute.nautilus.web4factory.Web4Action;
import institute.nautilus.web4factory.Web4Service;
import institute.nautilus.web4factory.Web4Connection;

@Component
public class Web4ActionEndpoint {
    static Logger logger = LoggerFactory.getLogger(Web4Connection.class);

	@ServiceActivator(inputChannel = "crypt")
	public void handleCrypt(Message<Web4Action> message) {
        Web4Action action = (Web4Action)message.getPayload();
        Object o = action.getArg(1);
        if (!(o instanceof String)){
            action.conn.send_fault("Entry underspecified");
            return;
        }
        action.conn.enable_crypt((String)o);
	}

    @ServiceActivator(inputChannel = "link")
    public void handleLink(Message<Web4Action> message) {
        Web4Action action = (Web4Action)message.getPayload();
        action.conn.link(action.argsToHostString());
    }

    @ServiceActivator(inputChannel = "access")
    public void handleAccess(Message<Web4Action> message) {
        Web4Action action = (Web4Action)message.getPayload();
        Web4Service service = (Web4Service)message.getHeaders().get("service");

        service.accessRoute(action.conn, action.argsToHostString());
    }

    @ServiceActivator(inputChannel = "data")
    public void handleData(Message<Web4Action> message) {
        Web4Action action = (Web4Action)message.getPayload();
        Object data = action.getArg(1);
        action.conn.setAccessData(data);
    }

	@ServiceActivator(inputChannel = "unkownAction")
    public void unkownAction(Message<?> message) {
        Web4Connection conn = (Web4Connection)message.getHeaders().get("connection");
        conn.send_fault("Unknown action");
    }


}
