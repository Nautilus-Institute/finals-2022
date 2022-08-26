package institute.nautilus.web4factory.web4factoryfactory;

import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.Web4FactoryExpression;
import institute.nautilus.web4factory.Web4FactoryException;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

public class Web4FactoryFactoryInputWidget extends Web4FactoryFactoryWidget {
    @Override
    public Web4FactoryExpression process() throws Web4FactoryException {
        return new Web4FactoryExpression("#input", String.class);
    }
}
