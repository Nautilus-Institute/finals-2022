package institute.nautilus.web4factory.web4factoryfactory;

import java.lang.*;
import java.util.*;
import java.io.*;

import org.apache.commons.text.StringEscapeUtils;

import institute.nautilus.web4factory.Web4FactoryExpression;
import institute.nautilus.web4factory.BoundedBufferedReader;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

public class Web4FactoryFactoryAwkWidget extends Web4FactoryFactoryWidget {

    public Web4FactoryFactoryAwkWidget(Object ... children) {
        super(children);
        this.name = "T(institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryAwkWidget).awk";
        this.argTypes = new Class[]{String.class, String.class};
        this.returnType = String.class;
    }

    public static String escape(String s) {
        return StringEscapeUtils.escapeEcmaScript(s);
    }

    public static Object grabLast() {
        String cmd = "{ print $NF }";
        return new Web4FactoryFactoryAwkWidget(cmd);
    }
    public static Object removeEmptyLines() {
        String cmd = "NF > 0 { print }";
        return new Web4FactoryFactoryAwkWidget(cmd);
    }
    public static Object numberLines() {
        String cmd = "{ print NR$0 }";
        return new Web4FactoryFactoryAwkWidget(cmd);
    }
    public static Object grabNth(int ind) {
        String cmd = String.format("{ print $%u }", ind);
        return new Web4FactoryFactoryAwkWidget(cmd);
    }
    public static Object joinLines(String join) {
        String cmd = String.format("{ printf $0\"%s\" }", escape(join));
        return new Web4FactoryFactoryAwkWidget(cmd);
    }
    public static Object prefixLines(String join) {
        String cmd = String.format("{ print \"%s\"$0 }", escape(join));
        return new Web4FactoryFactoryAwkWidget(cmd);
    }

    //awk(java.lang.String,java.lang.String)

    // XXX awk command injection
    public static String awk(String command, String in) {
        if (in.length() == 0) {
            return "";
        }

        Runtime rt = Runtime.getRuntime();
        String awk_cmd = escape(command);
        //String cmd = String.format("/usr/bin/timeout 5 /usr/bin/gawk $'%s' 2>/tmp/err2.log", awk_cmd);
        String cmd = String.format("/usr/bin/timeout 5 /usr/bin/gawk $'%s'", awk_cmd);
        //System.out.println("AWK CMD "+ cmd);
        String[] xcmd = new String[]{"bash","-c",cmd};
        Process p = null;
        try {
            p = rt.exec(xcmd);
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (p == null)
            return "error";

        PrintWriter pw = new PrintWriter(p.getOutputStream());

        if (in.endsWith("\n")) {
            pw.print(in);
        } else {
            pw.println(in);
        }
        pw.flush();
        pw.close();

        BoundedBufferedReader br = new BoundedBufferedReader(new InputStreamReader(p.getInputStream()), 100, 1000);


        String out = "";
        try {
            while (true) {
                String line = br.readLine();
                if (line == null)
                    break;
                //System.out.println("Read in: "+line);
                out += line + "\n";
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        //System.out.println("AWK RES "+ out);
        return out;
    }
}

