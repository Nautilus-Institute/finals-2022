package institute.nautilus.web4factory.web4factoryfactory;

import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactory;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.Web4FactoryExpression;
import institute.nautilus.web4factory.Web4FactoryException;

public class Web4FactoryFactory implements java.io.Serializable {
    static Logger logger = LoggerFactory.getLogger(Web4FactoryFactory.class);
    private static final long serialVersionUID = 31337;

    public Object factory;
    public Web4FactoryExpression factoryExpr;
    public String factoryName;

    public Web4FactoryFactory(String name, Object factory) {
        this.factoryName = name;
        this.factory = factory;
    }
    public void process() throws Web4FactoryException {
        if (this.factoryExpr != null) {
            return;
        }
        this.factoryExpr = Web4FactoryFactoryWidget.process(factory);
    }
    public Object feed(Object input) {
        //logger.info("Starting feed for {}", this.factoryExpr);
        return this.factoryExpr.exec(input);
    }

    public static Object d(String s) throws IOException, ClassNotFoundException{
        //logger.info("=== Got base64 to deserialize {}", s);
        byte[] data =  Base64.getDecoder().decode(s);
        final ByteArrayInputStream baistream = new ByteArrayInputStream(data);
        final ObjectInputStream oistream = new ObjectInputStream(baistream);
        Object obj = oistream.readObject();
        oistream.close();
        return obj;
    }

    public static Web4FactoryFactory from(String access_data) throws IOException, ClassNotFoundException {
        Object obj = d(access_data);

        if (!(obj instanceof Web4FactoryFactory)) {
            return null;
        }
        return (Web4FactoryFactory)obj;
    }

    public static String s(java.io.Serializable o) throws IOException{
        final ByteArrayOutputStream baostream = new ByteArrayOutputStream();
        final ObjectOutputStream oostream;

        oostream = new ObjectOutputStream(baostream);
        oostream.writeObject(o);
        oostream.close();
        return Base64.getEncoder().encodeToString(baostream.toByteArray());
    }

    @Override
    public String toString(){
        Web4FactoryExpression expr = this.factoryExpr;
        this.factoryExpr = null;
        String res = "bad";
        try {
            res = s(this);
        } catch (IOException e) {
            e.printStackTrace();
        }
        this.factoryExpr = expr;
        return res;
    }
}

