
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring> // Include the <cstring> header for memset

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // Include the <unistd.h> header for close

#include <ctime>
#include <fstream>
#include <dirent.h> // Include the <dirent.h> header for DIR, opendir, readdir, and closedir
#include <vector>

void sendResponse(int clientSocket, const std::string &response)
{
  std::string httpResponse = "HTTP/1.1 200 OK\r\n";
  httpResponse += "Content - Type:text/html\r\n";
  httpResponse += "Content - Length : " + std::to_string(response.length()) + "\r\n";
  httpResponse += "\r\n";
  httpResponse += "<html><head><title>T1 Lab SisOp</title></head><body>";
  httpResponse += response;
  httpResponse += "</body></html>";

  send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
  close(clientSocket);
}

std::string getCurrentTime()
{
  std::ifstream rtcFile("/proc/driver/rtc");
  std::string line;

  while (std::getline(rtcFile, line))
  {
    if (line.find("rtc_time") != std::string::npos)
    {
      std::string timeStr = line.substr(line.find(":") + 2);
      rtcFile.close();
      return "\n<h3>Tempo do sistema: " + timeStr + "</h3>";
    }
  }

  rtcFile.close();
  return "";
}

std::string getAfterDots(std::string line)
{
  return line.substr(line.find(":") + 2);
}

std::string getUptime()
{
  std::ifstream uptimeFile("/proc/uptime");
  if (!uptimeFile.is_open())
  {
    throw std::runtime_error("Unable to open /proc/uptime");
  }

  std::string uptime;
  uptimeFile >> uptime;

  uptimeFile.close();
  return "\n<h3>Tempo de funcionamento: " + uptime + "s </h3>";
}

std::string getStat()
{
    std::ifstream statFile("/proc/stat");
    if (!statFile.is_open())
    {
        throw std::runtime_error("Unable to open /proc/stat");
    }

    std::string line;
    std::string cpuStat;
    if (std::getline(statFile, line))
    {
        std::string cpuLine = line.substr(line.find("cpu") + 4); 
        std::istringstream ss(cpuLine);
        std::vector<std::string> tokens;
        std::string token;
        while (ss >> token)
        {
            tokens.push_back(token);
        }

        long user = std::stol(tokens[0]);
        long nice = std::stol(tokens[1]);
        long system = std::stol(tokens[2]);
        long idle = std::stol(tokens[3]);
        long total = user + nice + system + idle;

        cpuStat = std::to_string((total - idle) * 100 / total);
    }

    statFile.close();
    return "\n<h3>Uso da CPU: " + cpuStat + "% </h3>";
}



std::string getMemoryUsage()
{
  std::ifstream meminfoFile("/proc/meminfo");
  std::string line;
  int memoryTotal;
  int memoryUsage;

  while (std::getline(meminfoFile, line))
  {
    if (line.find("MemTotal:") != std::string::npos)
    {
      memoryTotal = std::stoi(getAfterDots(line));
    }
    else if (line.find("MemFree") != std::string::npos)
    {
      memoryUsage = memoryTotal - std::stoi(getAfterDots(line));
    }
  }

  meminfoFile.close();

  // Convert memory usage to MB
  return "<h3>Memoria total: " + std::to_string(memoryTotal / 1024) + "</h3>" +
         "<h3>Memoria usada:" + std::to_string(memoryUsage / 1024) + "</h3>";
  // int memoryUsageMB = std::stoi(memoryUsage) / 1024;
  // return "<p><h3>Memory Usage: </h3>" + std::to_string(memoryUsageMB) + " MB</p>";
}

std::string getCpuInfo()
{
  std::ifstream cpuInfoFile("/proc/cpuinfo");
  std::string line;
  std::string cpuModel;
  std::string cpuVelocity;

  while (std::getline(cpuInfoFile, line))
  {
    if (line.find("model name") != std::string::npos)
    {
      cpuModel = getAfterDots(line);
    }
    else if (line.find("cpu MHz") != std::string::npos)
    {
      cpuVelocity = getAfterDots(line);
    }
  }

  cpuInfoFile.close();
  return "<p><h3>CPU Model: </h3>" + cpuModel + ", <h3>CPU Velocity: </h3>" + cpuVelocity + "</p>";
}

std::string getSystemInfo()
{
  std::ifstream osReleaseFile("/etc/os-release");
  std::string line;
  std::string systemName;

  while (std::getline(osReleaseFile, line))
  {
    if (line.find("PRETTY_NAME") != std::string::npos)
    {
      std::string name = line.substr(line.find("=") + 1);
      systemName = name.substr(1, name.length() - 2);
    }
  }

  osReleaseFile.close();
  return "<p><h3>Sistema Operacional: " + systemName + " </h3>";
}

/*std::string getUSB()
{
    std::ifstream usbFile("/proc/bus/usb/devices");
    if (!usbFile.is_open())
    {
        throw std::runtime_error("Unable to open /proc/bus/usb/devices");
    }

    std::string line;
    std::string usbDevices;
    while (std::getline(usbFile, line))
    {
        usbDevices += line + "<br/>";
    }

    usbFile.close();
    return "<h3>Dispositivos USB conectados:</h3>" + usbDevices;
}*/


std::string getRunningProcesses()
{
  std::string processInfo = "<h3>Processos em execução:</h3>";

  // Open the /proc directory
  DIR *dir = opendir("/proc");
  if (dir == nullptr)
  {
    throw std::runtime_error("Unable to open /proc directory");
  }

  // Read the directory entries
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr)
  {
    // Check if the entry is a directory and its name is a number
    if (entry->d_type == DT_DIR && std::isdigit(entry->d_name[0]))
    {
      std::string pid = entry->d_name;

      // Open the status file for the process
      std::string statusFilePath = "/proc/" + pid + "/status";
      std::ifstream statusFile(statusFilePath);
      if (!statusFile.is_open())
      {
        continue;
      }

      std::string line;
      std::string processName;
      while (std::getline(statusFile, line))
      {
        if (line.find("Name:") != std::string::npos)
        {
          processName = line.substr(line.find(":") + 2);
        }
      }

      statusFile.close();

      // Append the process PID and name to the result string
      processInfo += "<ul><strong>PID:</strong> " + pid + ", <strong>Name:</strong> " + processName + "</ul>\n";
    }
  }

  closedir(dir);

  return processInfo;
}

std::string getAdptadores()
{
    std::ifstream routeFile("/proc/net/route");
    if (!routeFile.is_open())
    {
        throw std::runtime_error("Unable to open /proc/net/route");
    }

    std::string line;
    std::string adapters;
    while (std::getline(routeFile, line))
    {
        std::istringstream iss(line);
        std::string iface;
        std::string dest;
        std::string gateway;
        std::string flags;

        iss >> iface >> dest >> gateway >> flags;

        if (!iface.empty())
        {
            adapters += "<p><strong>Interface:</strong> " + iface + "</p>";
        }
    }

    routeFile.close();
    return "<h3>Adaptadores de rede:</h3>" + adapters;
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
      std::string response = "\n<h1> Welcome to the Simple Web Server !</ h1>";

      response += getCurrentTime();
      response += getUptime();
      response += getCpuInfo();
      response += getStat();
      response += getMemoryUsage();
      response += getSystemInfo();
      //response += getUSB();
      response += getRunningProcesses();
      response += getAdptadores();

      sendResponse(clientSocket, response);
    }
  }

  return 0;
}
