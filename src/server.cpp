#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring> // Include the <cstring> header for memset

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // Include the <unistd.h> header for close

void sendResponse(int clientSocket, const std::string &response)
{
  std::string httpResponse = "HTTP / 1.1 200 OK\r\n";
  httpResponse += "Content - Type : text/html\r\n";
  httpResponse += "Content - Length : " + std::to_string(response.length()) + "\r\n";
  httpResponse += "\r\n";
  httpResponse += response;

  send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
  close(clientSocket);
}

int main()
{
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  // Create server socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  // Configure server address
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8080);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  // Bind the server address to the socket
  bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

  if (listen(serverSocket, 5) == 0)
    std::cout << "Server started on port 8080…" << std::endl;
  else
    std::cout << "Failed to start the server !" << std::endl;

  addr_size = sizeof serverStorage;

  while (true)
  {
    // Accept client connection
    clientSocket = accept(serverSocket, (struct sockaddr *)&serverStorage, &addr_size);

    if (clientSocket >= 0)
    {
      // Receive client request
      char buffer[1024];
      memset(buffer, 0, sizeof(buffer));
      recv(clientSocket, buffer, sizeof(buffer), 0);

      // Process the request
      std::string request(buffer);
      std::string response = "<html><head><title> Simple Web Server</ title></ head>";
      response += "<body><h1> Welcome to the Simple Web Server !</ h1></ body></ html>";

      sendResponse(clientSocket, response);
    }
  }

  return 0;
}