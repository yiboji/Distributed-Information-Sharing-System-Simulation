import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.util.*;
class ServerThread implements Runnable{
	public static HashMap<String,String> map = new HashMap<>();
	private Socket client = null;  
	public ServerThread(Socket client){  
	    this.client = client;  
	}  
	@Override  
	public void run() {  
	    try{  
	        PrintStream out = new PrintStream(client.getOutputStream());  
	        BufferedReader buf = new BufferedReader(new InputStreamReader(client.getInputStream()));  
	        boolean f =true;  
	        while(f){  
	            String str =  buf.readLine();  
	            if(str == null || str.equals("")){  
	                break; 
	            }else{  
	                String response = "";
	                if(map.containsKey(str)) response = map.get(str);
	                else{
	                    UDPClient udp = new UDPClient(str,22887,2222);
	                    response = udp.getResponse();
	                    map.put(str, response);
	                }
	                out.println(response);	
	            }  
	        }  
	        out.close();  
	        client.close();  
	    }catch(Exception e){  
	        e.printStackTrace();  
	    }  
	}  
}