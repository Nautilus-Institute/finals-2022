package institute.nautilus.web4factory;

import java.lang.*;
import java.util.*;
import java.util.jar.*;
import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;
import org.apache.commons.io.IOUtils;

import co.libly.resourceloader.SharedLibraryLoader;
import co.libly.resourceloader.ResourceLoader;

import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import institute.nautilus.web4factory.Web4Connection;
import institute.nautilus.web4factory.BoundedBufferedReader;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactory;
import institute.nautilus.web4factory.web4factoryfactory.Web4FactoryFactoryWidget;

public class Web4FactoryParser implements java.io.Serializable {
    Object input = null;
    byte[] parseData;
    Object foobar = null;
    byte[] error;
    Object foobar2 = null;

    public Web4FactoryParser(String data) {
        data = data + "\0";
        this.input = data;
        this.parseData = data.getBytes(StandardCharsets.UTF_8);
        this.error = new byte[0x100];
    }

    public static void runParseLoop() {
        BoundedBufferedReader br = new BoundedBufferedReader(new InputStreamReader(System.in), 1024, 10000);
        LibFactory lf = new LibFactory();
        //System.err.println("Waiting for input");
        while (true) {
            String input = "";
            Object inobj = null;
            try {
                input = br.readLine();
                if (input == null) {
                    System.exit(0);
                }
                //System.err.println("Got input to parser: `"+ input+"`");
                inobj = Web4FactoryFactory.d(input);
            } catch (IOException e) {
                e.printStackTrace();
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }

            String out = "null";

            if (inobj != null) {
                //System.err.println("Got object `"+inobj.toString()+"`");
                Object f = lf.b(inobj);
                java.io.Serializable s = null;
                if (f == null && inobj instanceof Web4FactoryParser) {
                    s = ((Web4FactoryParser)inobj).error;
                } else if (f != null && f instanceof java.io.Serializable) {

                    s = (java.io.Serializable)f;
                }
                if (s != null) {
                    //System.err.println("Result is `"+s.toString()+"`");
                    try {
                        out = Web4FactoryFactory.s(s);
                    } catch (IOException e) {}
                }
            }

            //System.err.println("Sending output `"+out+"`");
            System.out.println(out);
            System.out.flush();
        }
    }
    public static void loadLibFactory() {
        try {
            //final List<Class> classes = new ArrayList<>();
            //classes.add(LibFactory.class);
            //URL f = new ResourceLoader().getThePathToTheJarWeAreIn(com.goterl.lazycode.lazysodium.Sodium.class);
            //logger.info("File path is {}", f);
            //URL f = ResourceLoader.getThePathToTheJarWeAreIn(com.goterl.lazycode.lazysodium.Sodium.class);
            //logger.info("File path is {}", f);
            //URL fullJarPathURL = ResourceLoader.getThePathToTheJarWeAreIn(LibFactory.class);
            String path = LibFactory.class.getProtectionDomain().getCodeSource().getLocation().getPath().split(":")[1];
            //System.out.println("Path is "+ path);
            path = path.split("!")[0];
            //System.out.println("Path is "+ path);

            File jar = new File(path);
            try {
                ZipInputStream zipInputStream = new ZipInputStream(new FileInputStream(path));
                ZipEntry entry = zipInputStream.getNextEntry();
                while (entry != null) {
                    String name = entry.getName();
                    if (name.indexOf(".so") != -1) {
                        //logger.info("Found entry {}", name);
                        Path filePath = Paths.get(path, entry.getName());

                        for (int i=0; i<100; i++) {
                            //logger.info("Cleaning /var/tmp/web4factory");
                            Web4Connection.cleanDir(new File("/var/tmp/web4factory"));
                            File outPath = File.createTempFile("tmp",".so", null);
                            try {
                                //BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(outPath));
                                IOUtils.copy(zipInputStream, new FileOutputStream(outPath));
                                /*
                                byte[] bytesIn = new byte[4096];
                                while(true) {
                                    int read = zipInputStream.read(bytesIn);
                                    logger.info("read {} bytes", read);
                                    if (read == -1)
                                        break;
                                    bos.write(bytesIn, 0, read);
                                }
                                */
                                //logger.info("Extracted to {}", outPath);
                                LibFactory.load(outPath.getAbsolutePath());
                                new LibFactory().a();
                                outPath.delete();
                                zipInputStream.closeEntry();
                                break;
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                        break;
                    }
                    zipInputStream.closeEntry();
                    entry = zipInputStream.getNextEntry();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }


            //SharedLibraryLoader.get().load("linux64/libfactory.so", classes);
        } catch (java.lang.UnsatisfiedLinkError e) {
            e.printStackTrace();
        } catch (java.lang.NoClassDefFoundError e){
            e.printStackTrace();
        } catch (Exception e){
            e.printStackTrace();
        }

    }

}
