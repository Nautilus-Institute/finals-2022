package institute.nautilus.web4factory.web4factoryfactory;

import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.Web4FactoryExpression;
import institute.nautilus.web4factory.Web4FactoryException;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

public class Web4FactoryFactoryConcatWidget extends Web4FactoryFactoryWidget {
    @Override
    public Web4FactoryExpression process() throws Web4FactoryException {
        int numArgs = children.size();
        String out = "";
        if (numArgs == 0)
            out = "''";
        for (int i=0; i<numArgs; i++) {
            Object o = children.get(i);
            Web4FactoryExpression expr = Web4FactoryFactoryWidget.process(o);
            out += Web4FactoryFactoryWidget.convert(expr, String.class).expr;
            if (i + 1 < numArgs)
                out += "+";
        }
        return new Web4FactoryExpression(out, String.class);
    }
}
