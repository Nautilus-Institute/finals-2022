package institute.nautilus.web4factory.web4factoryfactory;

import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.Web4FactoryExpression;
import institute.nautilus.web4factory.Web4FactoryException;

public class Web4FactoryFactoryWidget implements java.io.Serializable {
    private static final long serialVersionUID = 31337;

    public ArrayList<Object> children;

    public String name = null;
    public Class[] argTypes = null;
    public Class returnType = null;


    static Web4FactoryExpression process(Object c) throws Web4FactoryException {
        if (c == null) {
            return new Web4FactoryExpression("null", Object.class);
        }
        // TODO integers as well
        if (c instanceof Web4FactoryFactoryWidget) {
            Web4FactoryFactoryWidget n = (Web4FactoryFactoryWidget)c;
            return n.process();
        }
        return new Web4FactoryExpression(
            String.format("'%s'", c.toString()),
            String.class
        );
    }

    static Web4FactoryExpression convert(Web4FactoryExpression e, Class t) throws Web4FactoryException {
        if (e.type == t) {
            return e;
        }
        if (t == String.class) {
            return new Web4FactoryExpression(
                String.format("%s.toString()"),
                t
            );
        }
        throw new Web4FactoryException(String.format("Unable to convert arg to type %s", t.toString()));
    }

    public Web4FactoryFactoryWidget() {
        this.children = new ArrayList();
    }

    public Web4FactoryFactoryWidget(Object ... children) {
        this.children = new ArrayList(Arrays.asList(children));
    }

    public void addChildren(Object ... children) {
        this.children.addAll(Arrays.asList(children));
    }

    public void addChild(Object child) {
        this.children.add(child);
    }

    public Web4FactoryExpression process() throws Web4FactoryException {
        if (name != null) {
            int numArgs = 0;
            if (argTypes != null) {
                numArgs = argTypes.length;
            }
            if (children.size() < numArgs) {
                throw new Web4FactoryException(
                    String.format("Not enough args for `%s()`", this.name));
            }
            String out = this.name+"(";
            for (int i = 0; i<numArgs; i++) {
                Object o = children.get(i);
                Class t = argTypes[i];

                Web4FactoryExpression expr = Web4FactoryFactoryWidget.process(o);
                out += Web4FactoryFactoryWidget.convert(expr, t).expr;

                if (i + 1 < numArgs)
                    out += ",";
            }
            return new Web4FactoryExpression(out + ")", this.returnType);
        }
        if (children.size() == 0) {
            throw new Web4FactoryException("Child widget expected");
        }
        return Web4FactoryFactoryWidget.process(children.get(0));

    }
}

