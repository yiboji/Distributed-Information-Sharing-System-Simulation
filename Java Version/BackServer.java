import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;  
import java.net.DatagramPacket;  
import java.net.DatagramSocket;  
import java.util.*;
public class BackServer implements Runnable{  
	HashMap<String,String> map;
	int portno;
    public BackServer(String filename, int portno) throws IOException{  
    	this.portno = portno;
    	BufferedReader br = new BufferedReader(new FileReader(filename));
    	String line = br.readLine();
    	map = new HashMap<>();
    	while(line!=null){
    		String[] keyval = line.split(" ");
    		map.put(keyval[0], keyval[1]);
    		line = br.readLine();
    	}
    }
	@Override
	public void run() {
		// TODO Auto-generated method stub
		try{
	        byte[] buf = new byte[1024];  
	        DatagramSocket ds = new DatagramSocket(portno);  
	        DatagramPacket dp_receive = new DatagramPacket(buf, 1024);  
	        boolean f = true;  
	        while(f){
	            ds.receive(dp_receive);  
	            String str_receive = new String(dp_receive.getData()).trim();
	            String str_send;
	            if(map.containsKey(str_receive)){
	            	str_send = map.get(str_receive);
	            }
	            else{
	            	int nextport = 0;
	            	if(portno==22887){
	            		nextport = 23887;
	            	}
	            	UDPClient client = new UDPClient(str_receive,nextport,3333);
	            	str_send = client.getResponse();
	            	map.put(str_receive, str_send);
	            }
	            DatagramPacket dp_send= new DatagramPacket(str_send.getBytes(),str_send.length(),dp_receive.getAddress(),dp_receive.getPort());  
	            ds.send(dp_send);  
	            dp_receive.setLength(1024);  
	        }  
	        ds.close();  
		}catch(Exception e){
			e.printStackTrace();
		}
	}  
}  