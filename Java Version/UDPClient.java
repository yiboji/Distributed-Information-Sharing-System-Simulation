import java.io.InterruptedIOException;  
import java.net.DatagramPacket;  
import java.net.DatagramSocket;  
import java.net.InetAddress;

public class UDPClient{  
    private String response = null;
	public UDPClient(String message, int portno, int clientno) {
		// TODO Auto-generated method stub
       try {
    	   	String str_send = message;  
		    byte[] buf = new byte[1024];  
		    DatagramSocket ds = new DatagramSocket(clientno);  
		    InetAddress loc = InetAddress.getLocalHost();  
		    DatagramPacket dp_send= new DatagramPacket(str_send.getBytes(),str_send.length(),loc,portno);  
		    DatagramPacket dp_receive = new DatagramPacket(buf, 1024);  
		    ds.setSoTimeout(5000); 
		    ds.send(dp_send);  
		    try{  
		        ds.receive(dp_receive);  
		        response = new String(dp_receive.getData());  
			    dp_receive.setLength(1024);  
		    }catch(InterruptedIOException e){ 
		    	e.printStackTrace();
		    }  
		    ds.close();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}  
	}
	public String getResponse(){
		return response;
	}
}   