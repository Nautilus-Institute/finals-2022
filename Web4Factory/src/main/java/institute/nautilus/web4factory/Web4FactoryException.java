package institute.nautilus.web4factory;

import java.lang.*;
import java.util.*;
import java.io.*;

public class Web4FactoryException extends Exception {
    String msg;
    public Web4FactoryException(String msg) {
        this.msg = msg;
    }
}
