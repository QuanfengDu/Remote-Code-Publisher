///////////////////////////////////////////////////////////////////////
// MsgServer.cpp                                                    //
// ver 3.9                                                           //
// Language:    C++, Visual Studio 2015                              //
// Platform:    Dell XPS 13, Windows 10                              //
// Application:  Object Oriented Design                              //
// Author:      Quanfeng Du, Syracuse University,                    //
//              qdu101@syr.edu                                       //
///////////////////////////////////////////////////////////////////////

#include"MsgServer.h"
#include<sstream>
#include<string>
#include<stdio.h>
#include<windows.h>
//----< this defines processing to frame messages >------------------

HttpMessage ClientHandler::readMessage(Socket& socket)
{
  connectionClosed_ = false;
  HttpMessage msg;
  while (true) {
    std::string attribString = socket.recvString('\n');
    if (attribString.size() > 1) {
      HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
      msg.addAttribute(attrib);
    }
    else
      break;
  }
  if (msg.attributes().size() == 0){
    connectionClosed_ = true;
    return msg;
  }
  if (msg.attributes()[0].first == "POST"){
    std::string filename = msg.findValue("file");
    if (filename != "") {
      size_t contentSize;
      std::string sizeString = msg.findValue("content-length");
      if (sizeString != "")
        contentSize = Converter<size_t>::toValue(sizeString);
      else
        return msg;
      readFile(filename, contentSize, socket);
    }
    if (filename != "") {
      msg.removeAttribute("content-length");
      std::string bodyString = "<file>" + filename + "</file>";
      std::string sizeString = Converter<size_t>::toString(bodyString.size());
      msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
      msg.addBody(bodyString);
    }
    else{
      size_t numBytes = 0;
      size_t pos = msg.findAttribute("content-length");
      if (pos < msg.attributes().size())  {
        numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
        Socket::byte* buffer = new Socket::byte[numBytes + 1];
        socket.recv(numBytes, buffer);
        buffer[numBytes] = '\0';
        std::string msgBody(buffer);
        msg.addBody(msgBody);
        delete[] buffer;}
    }
  }
  return msg;
}
//----< read a binary file from socket and save >--------------------
/*
 * This function expects the sender to have already send a file message, 
 * and when this function is running, continuosly send bytes until
 * fileSize bytes have been sent.
 */
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
  std::string temp = FileSystem::Path::getName(filename,true);
  std::cout << "\n Server Received files: " << temp;
  std::string temp_ = FileSystem::Path::getName(filename, false);
  std::vector<std::string> direc = FileSystem::Directory::getDirectories();
  for (auto it : direc)
  {
	  if (it == temp_)
		  break;
	  else
	  {
		  std::string newfolder = "../Repository/" + temp_;
		  FileSystem::Directory::create(newfolder);
	  }
  }
  std::string fqname = "../Repository/"+temp_+"/" + temp;
  FileSystem::File file(fqname);
  file.open(FileSystem::File::out, FileSystem::File::binary);
  if (!file.isGood()) {
    Show::write("\n\n  can't open file " + fqname);
    return false;
  }
  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];
  size_t bytesToRead;
  while (true) {
    if (fileSize > BlockSize)
      bytesToRead = BlockSize;
    else
      bytesToRead = fileSize;
    socket.recv(bytesToRead, buffer);
    FileSystem::Block blk;
    for (size_t i = 0; i < bytesToRead; ++i)
      blk.push_back(buffer[i]);
    file.putBlock(blk);
    if (fileSize < BlockSize)
      break;
    fileSize -= BlockSize;
  }
  file.close();
  return true;
}
//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
	std::thread t1(
		[&]() {
		while (true)
		{
			HttpMessage msg = readMessage(socket);
			if (connectionClosed_ || msg.bodyString() == "quit"){
				Show::write("\n\n  clienthandler thread is terminating");
				break;
			}
			msgQ_.enQ(msg);
		}
	});
	std::thread t2(
		[&]() {
		while (true) {
			HttpMessage temp=msgQ_Send.deQ();
			std::string command = temp.findValue("command");
			std::cout << "\n\n Server received HTTPMessage: \n" << temp.toString();
			if (command != ""){
				if (command == "View all HTML")
					viewallhtml(socket);
				if (command == "download")
					setdownload(temp, socket);
				if (command == "delete")
					setdelete(temp);
				if (command == "category")
					setcategory(socket);
				if (command == "Details")
					setdetails(temp,socket);
				if (command == "connect")
					setconnect(socket);
				if (command == "ShowHtml")
					sendFile("allfiles.html", socket);
				if (command == "Dodepend")
					int i = system("..\\Debug\\CodeAnalyzer.exe ..\\Repository\\ *.cpp *.h *.cs");
			}
		}
	});
    t2.detach();
	t1.join();
}

//reaction if command is view all html
void ClientHandler::viewallhtml(Socket &socket)
{
	std::vector<std::string> files = FileSystem::Directory::getFiles("../HTML", "*.html");
	std::string filename_;
	for (size_t i = 0; i < files.size(); ++i)
	{
		filename_ = filename_ + files[i] + "\n";
	}
	std::cout << "\n View all files: "<<filename_;
	HttpMessage filesname = makeMessage(1, filename_, "localhost::8080");
	filesname.addAttribute(HttpMessage::attribute("command", "View all HTML"));
	sendMessage(filesname, socket);
}

//reaction if command is down load files
void ClientHandler::setdownload(HttpMessage temp, Socket& socket)
{
	std::string filebedown = temp.findValue("FileDL");
	std::vector<std::string> fdownload;
	for (size_t i = 0; i < filebedown.length(); i++)
	{
		if (filebedown[i] == ',')
			filebedown[i] = ' ';
	}
	std::stringstream ss(filebedown);
	std::string temp_;
	while (ss >> temp_)
		fdownload.push_back(temp_);
	std::vector<std::string> files = FileSystem::Directory::getFiles("../HTML", "*.html");
	for (auto it : fdownload)
	{
		for (auto iter : files)
		{
			if (iter.find(it) != std::string::npos)
			{
				std::cout << "\n Download file: "<<iter;
				sendFile(iter, socket);
				break;
			}
		}
	}

}

//reaction if command is selete files
void ClientHandler::setdelete(HttpMessage temp)
{
	std::string filebedown = temp.findValue("FileDL");
	std::vector<std::string> fdownload;
	for (size_t i = 0; i < filebedown.length(); i++)
	{
		if (filebedown[i] == ',')
			filebedown[i] = ' ';
	}
	std::vector<std::string> files = FileSystem::Directory::getFiles("../HTML", "*.html");
	std::stringstream ss(filebedown);
	std::string temp_;
	while (ss >> temp_)
		fdownload.push_back(temp_);

	for (auto it : fdownload)
	{
		for (auto iter : files)
		{
			if (iter.find(it) != std::string::npos)
			{
				iter = "../HTML/" + iter;
				std::cout << "\n been removed: " << iter;
				remove(iter.c_str());
				break;
			}
		}
	}
}

//reaction if command is show category
void ClientHandler::setcategory(Socket &socket)
{
	std::vector<std::string> cat = FileSystem::Directory::getDirectories("../Repository");
	std::string filename_;
	for (size_t i = 0; i < cat.size(); ++i)
	{
		filename_ = filename_ + cat[i] + "\n";
	}
	std::cout << "\n Show category: \n" << filename_;
	HttpMessage filesname = makeMessage(1, filename_, "localhost::8080");
	filesname.addAttribute(HttpMessage::attribute("command", "category"));
	sendMessage(filesname, socket);
}

//reaction if command is show fils under folder
void ClientHandler::setdetails(HttpMessage temp, Socket &socket)
{
	std::string folder = temp.findValue("Files_under_F");
	std::string path = "../Repository/" + folder;
	std::vector<std::string> fileunder = FileSystem::Directory::getFiles(path, "*.*");
	std::string filesunder;
	for (auto it : fileunder)
	{
		filesunder = filesunder + it + "\n";
	}
	std::cout << "\n All files under this folder: " << filesunder;
	HttpMessage filesname = makeMessage(1, filesunder, "localhost::8080");
	filesname.addAttribute(HttpMessage::attribute("command", "Details"));
	filesname.addAttribute(HttpMessage::attribute("Files_under_F", filesunder));
	sendMessage(filesname, socket);
}

//reaction if command is connect to server
void ClientHandler::setconnect(Socket &socket)
{
	std::cout << "\n Server received the connect message and connect with client\n";
	std::vector<std::string> files = FileSystem::Directory::getFiles("../HTML", "*.css");
	sendFile(files.at(0), socket);
	std::vector<std::string> files_ = FileSystem::Directory::getFiles("../HTML", "*.js");
	sendFile(files_.at(0), socket);
}

//----< HTTP message >----------------------------------
HttpMessage ClientHandler::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8081";  // ToDo: make this a member of the sender									 // given to its constructor.
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
		msg.addBody(body);
		if (body.size() > 0)
		{
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}

//----< read HTTP message >----------------------------------
void ClientHandler::sendMessage(HttpMessage& msg, Socket& socket)
{
	std::string msgString = msg.toString();
	std::cout << "\n\n Server send HTTPMessage:\n " << msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}

//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool ClientHandler::sendFile(const std::string& filename, Socket& socket)
{
	std::string fqname = "../HTML/" + filename;
	FileSystem::FileInfo fi(fqname);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	HttpMessage msg = makeMessage(1, "", "localhost::8080");
	msg.addAttribute(HttpMessage::Attribute("file", filename));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	sendMessage(msg, socket);
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}


//----< test stub >--------------------------------------------------

int main()
{
  ::SetConsoleTitle(L"HttpMessage Server - Runs Forever");
  Show::attach(&std::cout);
  Show::start();
  Show::title("\n  HttpMessage Server started");
  BlockingQueue<HttpMessage> msgQ;
  BlockingQueue<HttpMessage> msgQSend;
  try
  {
    SocketSystem ss;
    SocketListener sl(8080, Socket::IP6);
    ClientHandler cp(msgQ, msgQSend);
    sl.start(cp);
    while (true)
    {
      HttpMessage msg = msgQ.deQ();
	  HttpMessage msgSend = msg;
	  std::string tem=msgSend.findValue("command");
	  std::string tem2=msg.findValue("FileDL");
	  std::string tem3 = msg.findValue("Files_under_F");
	  std::string body = msgSend.bodyString();
	  msgSend = cp.makeMessage(1, body, "localhost::8080");
	  msgSend.addAttribute(HttpMessage::Attribute("command", tem));
	  msgSend.addAttribute(HttpMessage::Attribute("FileDL", tem2));
	  msgSend.addAttribute(HttpMessage::Attribute("Files_under_F", tem3));
	  msgQSend.enQ(msgSend);
    }
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}
