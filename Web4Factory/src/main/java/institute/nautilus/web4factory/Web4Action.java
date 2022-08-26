package institute.nautilus.web4factory;

import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.Web4Connection;

public class Web4Action {

    String name;
    ArrayList<Object> args;
    Web4Connection conn;

    public Web4Action(String name, Web4Connection conn) {
        // TODO check if this is a real action
        this.name = name;
        this.args = new ArrayList<Object>(2);
        this.conn = conn;
    }

    public void addArg(Object arg) {
        this.args.add(arg);
    }

    public Object getArg(int i) {
        i--;
        if (i >= args.size())
            return null;
        return this.args.get(i);

    }

    public String argsToHostString() {
        String res = "";
        for (int i=0; i<args.size(); i++) {
            if (args.get(i) instanceof String) {
                String s = (String)args.get(i);
                if (s.length() == 0)
                    continue;
                if (s.charAt(0) == '$') {
                    res = "/";
                    s = s.substring(1);
                    if (s.length() == 0)
                        continue;
                }
                if (i != 0 && !res.equals("/"))
                    res += "/";
                res += s;
            }
        }
        return res;
    }
}
