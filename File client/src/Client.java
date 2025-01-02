import java.io.*;
import java.net.Socket;
import javax.swing.JFileChooser;

public class Client {

    private static DataOutputStream dataOutputStream = null;
    private static DataInputStream dataInputStream = null;

    public static void main(String[] args) {
        try (Socket socket = new Socket("localhost", 900)) {
            dataOutputStream = new DataOutputStream(socket.getOutputStream());
            dataInputStream = new DataInputStream(socket.getInputStream());

            System.out.println("Select a file or folder to send to the server.");
            String path = selectFileOrFolder();
            if (path != null) {
                File fileOrFolder = new File(path);
                if (fileOrFolder.isDirectory()) {
                    sendFolder(fileOrFolder);
                } else {
                    sendFile(fileOrFolder);
                }
            } else {
                System.out.println("No file or folder selected. Exiting...");
            }

            dataInputStream.close();
            dataOutputStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Opens a file chooser dialog for selecting a file or folder.
     * 
     * @return The selected file or folder's absolute path, or null if no selection is made.
     */
    private static String selectFileOrFolder() {
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
        int result = fileChooser.showOpenDialog(null);
        if (result == JFileChooser.APPROVE_OPTION) {
            return fileChooser.getSelectedFile().getAbsolutePath();
        }
        return null;
    }

    /**
     * Sends a single file to the server.
     * 
     * @param file The file to send.
     * @throws Exception If an error occurs during file transmission.
     */
    private static void sendFile(File file) throws Exception {
        dataOutputStream.writeUTF("FILE");
        dataOutputStream.writeUTF(file.getName());
        dataOutputStream.writeLong(file.length());

        FileInputStream fileInputStream = new FileInputStream(file);
        byte[] buffer = new byte[4 * 1024];
        int bytes;
        while ((bytes = fileInputStream.read(buffer)) != -1) {
            dataOutputStream.write(buffer, 0, bytes);
        }

        fileInputStream.close();
        System.out.println("File sent: " + file.getName());
    }

    /**
     * Sends a folder and its contents recursively to the server.
     * 
     * @param folder The folder to send.
     * @throws Exception If an error occurs during folder transmission.
     */
    private static void sendFolder(File folder) throws Exception {
        dataOutputStream.writeUTF("FOLDER");
        dataOutputStream.writeUTF(folder.getName());

        for (File file : folder.listFiles()) {
            if (file.isDirectory()) {
                sendFolder(file); // Recursively send subfolders
            } else {
                sendFile(file);
            }
        }

        dataOutputStream.writeUTF("END_OF_FOLDER"); // Indicate end of folder contents
        System.out.println("Folder sent: " + folder.getName());
    }
}
