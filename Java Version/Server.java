import java.io.IOException;
import java.io.PrintWriter;
import java.net.DatagramSocket;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;

public class Server implements Runnable{
	ServerSocket tcpServerListener;
	DatagramSocket udpClient;
	Thread t;
	String threadName;
	public Server(int portno, String serverName) throws IOException{
		tcpServerListener = new ServerSocket(portno);
		this.threadName = serverName;
	}
	public void start(){
		t = new Thread(this, this.threadName);
		t.start();
	}
	@Override
	public void run(){
		try {
			// TODO Auto-generated method stub
			while(true){
				Socket socket = tcpServerListener.accept();
				try {
					PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
					out.println(new Date().toString());
				} finally {
					socket.close();
				}
			}
		} catch(IOException e){
			e.printStackTrace();
		}
		finally {
			// TODO: handle finally clause
			try {
				tcpServerListener.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}
