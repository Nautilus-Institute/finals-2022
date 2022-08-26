package institute.nautilus.web4factory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHeaders;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.beans.factory.annotation.Autowired;
import org.apache.commons.lang.StringUtils;

import com.sun.jna.Native;
import org.apache.commons.io.IOUtils;

import com.google.gson.*;
import com.google.gson.stream.*;
import com.goterl.lazycode.lazysodium.SodiumJava;
import com.goterl.lazycode.lazysodium.utils.Key;
import com.goterl.lazycode.lazysodium.utils.KeyPair;
import com.goterl.lazycode.lazysodium.interfaces.Box;
import com.goterl.lazycode.lazysodium.LazySodium;
import com.goterl.lazycode.lazysodium.LazySodiumJava;
import com.goterl.lazycode.lazysodium.exceptions.SodiumException;


import java.lang.*;
import java.util.*;
import java.util.jar.*;
import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermissions;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.charset.StandardCharsets;

import co.libly.resourceloader.SharedLibraryLoader;
import co.libly.resourceloader.ResourceLoader;
//import co.libly.resourceloader.WResourceLoader;

import institute.nautilus.web4factory.BoundedBufferedReader;
import institute.nautilus.web4factory.Web4Action;
import institute.nautilus.web4factory.Web4Parser;
import institute.nautilus.web4factory.Web4Service;
import institute.nautilus.web4factory.Web4Gateway;
import institute.nautilus.web4factory.Web4FactoryApplication;
import institute.nautilus.web4factory.LibFactory;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactory;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

public class Web4Connection {
    static SodiumJava sodium = null;;

    static Logger logger = LoggerFactory.getLogger(Web4Connection.class);
    Gson gson = new Gson();

    MessageChannel reply_channel = null;
    LazySodiumJava lazySodium = null;
    boolean isEncrypted = false;
    KeyPair keypair;
    Key clientKey = null;
    Object loadedData = null;
    String host_path = "/";

    Web4FactoryFactory factory = null;
    Process libfactory = null;
    BoundedBufferedReader libfactoryInput = null;
    PrintWriter libfactoryOutput = null;

    public Web4Connection() {
        this.loadSodium();
    }

    static Set<PosixFilePermission> writable_perm = PosixFilePermissions.fromString("rwxrwx---");

    public static void cleanDir(File dir) {
        try {
            for (File file: dir.listFiles()) {
                Files.setPosixFilePermissions(file.toPath(), writable_perm);
                if (file.isDirectory()) {
                    cleanDir(file);
                }
                file.delete();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }


    public static void loadSodium() {
        SodiumJava sj = Web4Connection.sodium;
        //logger.info("Cleaning /var/tmp/web4factory");
        cleanDir(new File("/var/tmp/web4factory"));
        //logger.info("Loading libsodium");
        try {
            if (sj == null) {
                sj = new SodiumJava("/usr/lib/x86_64-linux-gnu/libsodium.so.23");
            }
        } catch (java.lang.UnsatisfiedLinkError e) {
            e.printStackTrace();
        } catch (java.lang.NoClassDefFoundError e){
            e.printStackTrace();
        } catch (Exception e){
            e.printStackTrace();
        }
        Web4Connection.sodium = sj;
    }

    public Object getAccessData() {
        Object o = this.loadedData;
        this.loadedData = null;
        return o;
    }
    public void setAccessData(Object data) {
        this.loadedData = data;
    }

    public void setReplyChannel(MessageChannel reply) {
        this.reply_channel = reply;
    }

    String json_param(Object v) {
        return "#"+this.gson.toJson(v);
    }

    String host_with_path(String path) {
        String res = this.host_path;
        if (path.charAt(0) == '/') {
            res = path;
        } else if (this.host_path.equals("/")) {
            res += path;
        } else {
            res += "/" + path;
        }
        return res;
    }

    public void link(String path) {
        this.host_path = host_with_path(path);
        //logger.info("Host path is now {}", this.host_path);
    }

    public void enable_crypt(String key) {
        if (sodium == null) {
            loadSodium();
        }
        if (sodium == null) {
            this.send_fault("Failed to enable crypt");
            return;
        }
        try {
            if (this.lazySodium == null) {
                this.lazySodium = new LazySodiumJava(sodium, StandardCharsets.UTF_8);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (this.lazySodium == null) {
            this.send_fault("Failed to enable crypt");
            return;
        }
        try {
            this.clientKey = Key.fromHexString(key);
        } catch(Exception e) {
            this.send_fault("Failed to decode key");
            return;
        }
        if (clientKey == null) {
            this.send_fault("Failed to decode key");
            return;
        }
        try {
            this.keypair = this.lazySodium.cryptoBoxKeypair();
            String my_pub = this.keypair.getPublicKey().getAsHexString();
            this.send_response(String.format("crypt@%s", my_pub));
            //logger.info("Switching to encrypted protocol");
            this.isEncrypted = true;
        } catch (SodiumException e) {
            e.printStackTrace();
            this.send_fault("Crypt failed");
        }
    }

    public void send_response(String msg) {
        //logger.info("Responding with {} Encrypted: {}", msg, this.isEncrypted);

        byte[] msg_bytes = msg.getBytes(StandardCharsets.UTF_8);
        if (this.isEncrypted) {
            try {
                byte[] keyBytes = this.clientKey.getAsBytes();
                byte[] cipher = new byte[Box.SEALBYTES + msg_bytes.length];

                if (!this.lazySodium.cryptoBoxSeal(cipher, msg_bytes, msg_bytes.length, keyBytes)) {
                    throw new SodiumException("Could not encrypt message.");
                }

                String encrypted = Base64.getEncoder().encodeToString(cipher);



                //String encrypted = this.lazySodium.cryptoBoxSealEasy(msg, this.keypair.getPublicKey());
                //logger.info("encrypted is {}",encrypted);
                msg_bytes = encrypted.getBytes(StandardCharsets.UTF_8);
            } catch (SodiumException e) {
                e.printStackTrace();
            }

            //byte[] output = new byte*
            //this.lazySodium.cryptoBoxSeal(

            //this.reply_channel.send(out);
        }
        Message<?> out = MessageBuilder.withPayload(msg_bytes).build();
        this.reply_channel.send(out);
        this.reply_channel = null;
    }

    public void send_fault(Object msg) {
        if (this.reply_channel == null)
            return;
        this.send_response(String.format("fault%s", json_param(msg)));
    }

    public void send_data(Object msg) {
        if (this.reply_channel == null)
            return;
        this.send_response(String.format("data%s", json_param(msg)));
    }


    public Web4Action process_line(String line) throws java.io.IOException {
        if (this.isEncrypted) {
            try {
                byte[] cipher = Base64.getDecoder().decode(line);
                if (cipher.length <= Box.SEALBYTES) {
                    this.send_fault("Decrypt failed");
                    return null;
                }
                    
                byte[] message = new byte[cipher.length - Box.SEALBYTES];

                if (!this.lazySodium.cryptoBoxSealOpen(
                        message, cipher, cipher.length, 
                        this.keypair.getPublicKey().getAsBytes(),
                        this.keypair.getSecretKey().getAsBytes())
                ) {
                    throw new SodiumException("Could not decrypt message.");
                }

                line = new String(message);
            } catch (Exception e) {
                e.printStackTrace();
                this.send_fault("Decrypt failed");
                return null;
            }

            //byte[] output = new byte*
            //this.lazySodium.cryptoBoxSeal(

            //this.reply_channel.send(out);
        }
        //logger.info("Connection sent {}", line);

        Web4Parser p = new Web4Parser(line, this);
        Web4Action action = p.parse();
        //logger.info("Web4 action is {}", action);

        if (action == null) {
            this.send_fault("Unknown action");
            return null;
        }

        //Message<?> actionMsg = MessageBuilder.withPayload(action).build();
        //this.web4Gateway.print(actionMsg);
        /*

        if (action.name.equals("crypt")) {
            Object o = action.getArg(1);
            if (!(o instanceof String)){
                this.send_fault("Entry underspecified");
                return false;
            }
            this.handle_crypt((String)o);
            return true;
        }
        this.send_fault("Unkown action");
        */
        return action;
    }

    public void libfactory_destroy() {
        if (this.libfactory == null) {
            return;
        }
        try {
            this.libfactory.exitValue();
        } catch (IllegalThreadStateException e) { }
        this.libfactory.destroy();
        this.libfactory = null;
        this.libfactoryInput = null;
        this.libfactoryOutput = null;
    }

    public Web4FactoryFactory loadFactory(String data) {
        Web4FactoryFactory f = null;
        try {
            Object o = Web4FactoryFactory.d(data);
            if (o instanceof Web4FactoryFactory) {
                f = (Web4FactoryFactory)o;
            } else if (o instanceof byte[]) {
                String s = new String((byte[])o);

                s = org.apache.commons.lang.StringUtils.strip(s, "\0");
                this.send_fault(String.format("Error Parsing: %s", s));
                return null;
            } else {
                this.send_fault("Got unexpected object from parser");
                return null;
            }
        } catch(IOException e) {
            e.printStackTrace();
            this.send_fault("Failed to load factory");
            return null;
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            this.send_fault("Factory using unknown class");
            return null;
        }
        if (f == null) {
            this.send_fault("Expected Web4FactoryFactory object");
            return null;
        }
        try {
            f.process();
        } catch (Web4FactoryException e) {
            e.printStackTrace();
            this.send_fault(e.msg);
            return null;
        }
        //logger.info("Expression is {}", f.factoryExpr.expr);
        this.factory = f;
        return f;
    }

    public Web4FactoryFactory libfactory_parse(Object data_obj) {
        if (data_obj instanceof String) {
            data_obj = new Web4FactoryParser((String)data_obj);
        }

        if (data_obj instanceof java.io.Serializable) {}
        else {
            return null;
        }
        java.io.Serializable data = (java.io.Serializable)data_obj;

        if (this.libfactory != null) {
            try {
                this.libfactory.exitValue();
                this.libfactory_destroy();
            } catch (IllegalThreadStateException e) { }
        }
        if (this.libfactory == null) {
            Runtime rt = Runtime.getRuntime();

            String path = LibFactory.class.getProtectionDomain().getCodeSource().getLocation().getPath().split(":")[1];
            //System.out.println("Path is "+ path);
            path = path.split("!")[0];


            try {
                //String cmd = "/usr/bin/timeout 10 /usr/bin/java -cp " + path + " -Dloader.main=institute.nautilus.web4factory.Web4Gateway org.springframework.boot.loader.PropertiesLauncher 2>/tmp/err.log";
                //String cmd = "/usr/bin/timeout 10 /usr/bin/java -cp " + path + " -Dloader.main=institute.nautilus.web4factory.Web4Gateway org.springframework.boot.loader.PropertiesLauncher 2>/tmp/err.log";
                //String cmd = "/usr/bin/timeout -s 9 20 /usr/bin/java -Djava.io.tmpdir=/var/tmp/web4factory/ -XX:-UsePerfData -jar " + path + " parser";
                String cmd = "/usr/bin/timeout -s 9 20 /usr/bin/java -Djava.io.tmpdir=/var/tmp/web4factory/ -XX:-UsePerfData -jar " + path + " parser 2>/tmp/err.log";
                String[] xcmd = new String[]{"bash","-c",cmd};
                logger.info("Running command {}", cmd);
            this.libfactory = rt.exec(xcmd);

            logger.info("Started libfactory process {}", this.libfactory);
            this.libfactoryInput = new BoundedBufferedReader(new InputStreamReader(this.libfactory.getInputStream()), 1024, 10000);
            this.libfactoryOutput = new PrintWriter(this.libfactory.getOutputStream());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        if (this.libfactory == null) {
            return null;
        }
        String encoded = "";
        try {
            encoded = Web4FactoryFactory.s(data);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        //String encoded = Base64.getEncoder().encodeToString(data.getBytes());

        //logger.info("PPPPPPPPPPPPPPPPPP Sending data `{}`", encoded);
        this.libfactoryOutput.println(encoded);
        this.libfactoryOutput.flush();

        //this.libfactory_destroy();
        //return null


        String res_data = "";
        try {
            res_data = this.libfactoryInput.readLine();
            if (res_data == null) {
                this.libfactory_destroy();
                return null;
            }
            //logger.info("PPPPPPPPPPPPPPPPPP Got data `{}`", res_data);
        } catch (IOException e) {
            this.libfactory_destroy();
            return null;
        }
        try {
            return this.loadFactory(res_data);
        } catch (IllegalArgumentException e) {
            return null;
        }
    }
}
