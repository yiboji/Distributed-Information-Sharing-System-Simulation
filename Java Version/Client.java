import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;  
import java.io.InputStreamReader;  
import java.io.PrintStream;  
import java.net.Socket;  
import java.net.SocketTimeoutException;
import java.util.HashMap;  

public class Client {  
    public static void main(String[] args) throws IOException {  
    	BufferedReader br = new BufferedReader(new FileReader("client.txt"));
		String line = br.readLine();
		HashMap<String, String> map = new HashMap<>();
		while(line!=null){
			String[] keyval = line.split(" ");
			map.put(keyval[0], keyval[1]);
			line = br.readLine();
		}
        Socket client = new Socket("127.0.0.1", 21887);  
        client.setSoTimeout(10000);  
        BufferedReader input = new BufferedReader(new InputStreamReader(System.in));  
        PrintStream out = new PrintStream(client.getOutputStream());  
        BufferedReader buf =  new BufferedReader(new InputStreamReader(client.getInputStream()));  
        boolean flag = true;
        while(flag){  
            System.out.print("Please Enter：");  
            String str = input.readLine();  
            out.println(map.get(str));  
	        try{  
	            String echo = buf.readLine();  
	            System.out.println(echo);  
	        }catch(SocketTimeoutException e){  
	            System.out.println("TCP Time out");  
	        }  
        }  
        input.close();  
        if(client != null){  
            client.close();  
        }  
    }  
}  