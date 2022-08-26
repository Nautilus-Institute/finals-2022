package institute.nautilus.web4factory;

import java.security.Permission;
import java.util.Scanner;
import java.lang.*;
import java.util.*;
import java.io.*;

public class Web4SecurityManager extends SecurityManager {
    @Override
    public void checkPermission( Permission permission ) {
        //System.out.println("In check Permission");

    }
    @Override
    public void checkExit(int status) {
        throw new SecurityException("Service tried to exit");
    }

}
