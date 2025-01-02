import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {

    private static DataOutputStream dataOutputStream = null;
    private static DataInputStream dataInputStream = null;

    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(900)) {
            System.out.println("Server is Starting on Port 900");

            Socket clientSocket = serverSocket.accept();
            System.out.println("Client Connected");

            dataInputStream = new DataInputStream(clientSocket.getInputStream());
            dataOutputStream = new DataOutputStream(clientSocket.getOutputStream());

            while (true) {
                String type = dataInputStream.readUTF(); // Read type (FILE/FOLDER)
                if (type.equals("FILE")) {
                    receiveFile();
                } else if (type.equals("FOLDER")) {
                    receiveFolder();
                } else {
                    break; // Exit loop if transmission ends
                }
            }

            dataInputStream.close();
            dataOutputStream.close();
            clientSocket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Receives a single file from the client and saves it.
     * 
     * @throws Exception If an error occurs during file reception.
     */
    private static void receiveFile() throws Exception {
        String fileName = dataInputStream.readUTF();
        long fileSize = dataInputStream.readLong();

        File file = new File("Received/" + fileName);
        file.getParentFile().mkdirs(); // Create parent directories if they don't exist

        FileOutputStream fileOutputStream = new FileOutputStream(file);
        byte[] buffer = new byte[4 * 1024];
        int bytes;
        while (fileSize > 0 && (bytes = dataInputStream.read(buffer, 0, (int) Math.min(buffer.length, fileSize))) != -1) {
            fileOutputStream.write(buffer, 0, bytes);
            fileSize -= bytes;
        }

        fileOutputStream.close();
        System.out.println("File received: " + fileName);
    }

    /**
     * Receives a folder and its contents recursively from the client.
     * 
     * @throws Exception If an error occurs during folder reception.
     */
    private static void receiveFolder() throws Exception {
        String folderName = dataInputStream.readUTF();
        File folder = new File("Received/" + folderName);
        folder.mkdirs(); // Create the folder

        while (true) {
            String type = dataInputStream.readUTF(); // Check if file or folder ends
            if (type.equals("FILE")) {
                receiveFile();
            } else if (type.equals("END_OF_FOLDER")) {
                break; // End of this folder's contents
            }
        }

        System.out.println("Folder received: " + folderName);
    }
}
