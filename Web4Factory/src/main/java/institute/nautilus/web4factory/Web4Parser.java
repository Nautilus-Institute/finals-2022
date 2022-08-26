package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.google.gson.*;
import com.google.gson.stream.*;

import java.nio.charset.StandardCharsets;
import java.lang.*;
import java.util.*;
import java.io.*;

import institute.nautilus.web4factory.BoundedBufferedReader;
import institute.nautilus.web4factory.Web4Action;
import institute.nautilus.web4factory.Web4Connection;

public class Web4Parser {
    static Logger logger = LoggerFactory.getLogger(Web4Parser.class);
    Gson gson = new Gson();

    BufferedReader br;
    Web4Connection con;

    public Web4Parser(String line, Web4Connection conn) {
        //logger.info("Parsing web4 action {}", line);
        this.br = new BufferedReader(new StringReader(line));
        this.con = conn;
    }

    void error(String msg) {
        this.con.send_fault(msg);
    }

    public Web4Action parse() throws java.io.IOException {
        BufferedReader br = this.br;
        String act = this.get_name(32);
        //logger.info("Action is {}", act);

        if (act == "") {
            this.error("Unkown action");
            return null;
        }

        this.skip_space();

        Web4Action action = new Web4Action(act, this.con);
        ArrayList<Object> args = new ArrayList<Object>(2);

        while (br.ready()) {
            int c = br.read();
            //logger.info("Parsing args, got char `{}` {}", (char)c, c);
            if (c == -1) break;

            Object arg = null;
            if (c == '@') {
                arg = this.get_name(100);
            } else if (c == '#') {
                try {
                    arg = this.parse_next_json();
                } catch(Exception e) {
                    e.printStackTrace();
                    this.error("Error parsing, invalid arguments");
                    return null;
                }
                //arg = this.gson.fromJson(br, Object.class);
            } else {
                String rest = br.readLine();
                //logger.info("bad char {} rest is {}", c, rest);
                this.error("Error parsing, expected argument");
                return null;
            }
            //logger.info("Adding arg {}", arg);
            action.addArg(arg);
            this.skip_space();
        }
        //logger.info("Args are `{}`", action.args);

        //logger.info("Done parsing action");

        return action;
    }

    int peek_char() throws java.io.IOException {
        this.br.mark(2);
        int c = this.br.read();
        this.br.reset();
        return c;
    }

    void skip_space() throws java.io.IOException {
        BufferedReader br = this.br;
        while(br.ready()) {
            int c = this.peek_char();
            if (Character.isWhitespace(c)) {
                //logger.info("Skipping char {}", c);
                br.skip(1);
            } else {
                break;
            }
        };
        //logger.info("Skip space done");
    }

    String get_name(int max_len) throws java.io.IOException {
        BufferedReader br = this.br;
        br.mark(max_len);
        int i;
        for (i=0; i < max_len && br.ready() ;i++) {
            int c = br.read();
            if (c == -1) {
                break;
            }

            if (Character.isLetterOrDigit(c)) {
                continue;
            }
            if (c == '_' || c == '-' || c == '$' || c == '%') {
                continue;
            }
            break;
        }
        if (i == 0)
            return "";

        char[] out = new char[i];
        br.reset();
        br.read(out, 0, i);
        return new String(out);
    }


    Object parse_next_json() throws java.io.IOException {
        BufferedReader br = this.br;
        StringBuilder b = new StringBuilder();
        Stack<Integer> st = new Stack<Integer>();
        st.push(0); // 0 = Top level
                    // 1 = In object
                    // 2 = In list
                    // 3 = In string

        boolean escaped = false;
        while (br.ready()) {
            int c = this.peek_char();
            if (c == -1) break;

            //logger.info("Checking for json got {} `{}` in state {}", c, (char)c, st.peek());

            boolean is_ws = Character.isWhitespace(c);
            if (escaped) {
                //logger.info("Consuming char `{}` because it was escaped", (char)c);
                b.append((char)c);
                br.skip(1);
                escaped = false;
                continue;
            }

            int state = 0;
            if (c == '@' || c == '#' || is_ws) {
                state = st.peek();
                if (state == 0) {
                    int c2 = this.peek_char();
                    //logger.info("Done with parsing json, not consuming char. Next should be `{}`", (char)c2);

                    // Done with json
                    break;
                }

                b.append((char)c);
                br.skip(1);
                continue;
            }

            //logger.info("Consuming char `{}`", (char)c);
            b.append((char)c);
            br.skip(1);

            if (c == '{') {
                state = st.peek();
                if (state != 3) {
                    st.push(1);
                }
                continue;
            }
            if (c == '}') {
                state = st.peek();
                if (state == 1) {
                    st.pop();
                }
                continue;
            }
            if (c == '[') {
                state = st.peek();
                if (state != 3) {
                    st.push(2);
                }
                continue;
            }
            if (c == ']') {
                state = st.peek();
                if (state == 2) {
                    st.pop();
                }
                continue;
            }
            if (c == '"') {
                state = st.peek();
                if (state == 3) {
                    st.pop();
                } else {
                    st.push(3);
                }
                continue;
            }
            if (c == '\\') {
                state = st.peek();
                if (state == 3) {
                    escaped = true;
                }
                continue;
            }
        }

        String json_to_parse = b.toString();
        //logger.info("Going to parse {} bytes: `{}`", json_to_parse.length(), json_to_parse);
        Object res = this.con.gson.fromJson(json_to_parse, Object.class);
        if (res instanceof String) {
            String s = (String)res;
            if (s.indexOf(0) != -1) {
                res = s.getBytes(StandardCharsets.UTF_8);
            }
        }
        //logger.info("Parser got {}",res);
        return res;
    }

}
