import java.io.BufferedReader;
import java.io.FileReader;
import java.net.ServerSocket;  
import java.net.Socket;  
import java.util.*;

public class FrontServer {
    public static void main(String[] args) throws Exception{  
		BufferedReader br = new BufferedReader(new FileReader("server1.txt"));
		String line = br.readLine();
		HashMap<String, String> map = ServerThread.map;
		while(line!=null){
			String[] keyval = line.split(" ");
			map.put(keyval[0], keyval[1]);
			line = br.readLine();
		}
        ServerSocket server = new ServerSocket(21887);  
        Socket client = null;  
        new Thread(new BackServer("server2.txt",22887)).start();
        new Thread(new BackServer("server3.txt",23887)).start();

        boolean f = true;
        while(f){
            client = server.accept();  
            new Thread(new ServerThread(client)).start();  
        }  
        server.close();  
    }
}
