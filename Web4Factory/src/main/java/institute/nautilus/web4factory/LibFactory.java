package institute.nautilus.web4factory;

import java.lang.*;
import java.util.*;
import java.io.*;

import java.nio.file.Paths;

public class LibFactory {
    static void load(String path) {
        /*
        String path = LibFactory.class.getProtectionDomain().getCodeSource().getLocation().getPath().split(":")[1];
        System.out.println("Path is "+ path);
        path = path.split("!")[0];
        System.out.println("Path is "+ path);

        path = Paths.get(path).getParent().toString();
        System.out.println("Path is " + path);
        */
        System.load(path);
    }


    public int read_fd;
    public int write_fd;

    public native boolean a();
    public native Object b(Object a); //send();
}
