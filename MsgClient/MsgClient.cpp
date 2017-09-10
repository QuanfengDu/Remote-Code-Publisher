///////////////////////////////////////////////////////////////////////
// MsgServer.h                                                       //
// ver 3.9                                                           //
// Language:    C++, Visual Studio 2015                              //
// Platform:    Dell XPS 13, Windows 10                              //
// Application:  Object Oriented Design                              //
// Author:      Quanfeng Du, Syracuse University,                    //
//              qdu101@syr.edu                                       //
///////////////////////////////////////////////////////////////////////

#include"MsgClient.h"


//----< factory for creating messages >------------------------------
/*
 * This function only creates one type of message for this demo.
 * - To do that the first argument is 1, e.g., index for the type of message to create.
 * - The body may be an empty string.
 * - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
 *   expects the receiver EndPoint for the toAddr attribute.
 */

size_t ClientCounter::clientCount = 0;

//----< make message using socket >----------------------------------
HttpMessage MsgClient::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
  HttpMessage msg;
  HttpMessage::Attribute attrib;
  EndPoint myEndPoint = "localhost:8081";  // ToDo: make this a member of the sender
                                           // given to its constructor.
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


//----< read message using socket >-------------------------------------
HttpMessage MsgClient::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;
	while (true){
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1){
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
		if (filename != ""){
			size_t contentSize;
			std::string sizeString = msg.findValue("content-length");
			if (sizeString != "")
				contentSize = Converter<size_t>::toValue(sizeString);
			else
				return msg;
			readFile(filename, contentSize, socket);
		}
		if (filename != ""){
			msg.removeAttribute("content-length");
			std::string bodyString = "<file>" + filename + "</file>";
			std::string sizeString = Converter<size_t>::toString(bodyString.size());
			msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
			msg.addBody(bodyString);
		}
		else{
			size_t numBytes = 0;
			size_t pos = msg.findAttribute("content-length");
			if (pos < msg.attributes().size()){
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


//----< send message using socket >-------------------------------------
void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
{
  std::string msgString = msg.toString();
  socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}
//----< send file using socket >-------------------------------------
/*
 * - Sends a message to tell receiver a file is coming.
 * - Then sends a stream of bytes until the entire file
 *   has been sent.
 * - Sends in binary mode which works for either text or binary.
 */
bool MsgClient::sendFile(const std::string& filename, Socket& socket)
{
	if (filename.find(".cpp") != std::string::npos || filename.find(".h") != std::string::npos)
	{
		std::string fqname = /*"../TestFiles/" +*/ filename;
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
	else
		return false;
}
//----< this defines the behavior of the client >--------------------

//----< read file using socket >-------------------------------------
bool MsgClient::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
	std::string fqname = "../Result/" + filename;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		Show::write("\n\n  can't open file " + fqname);
		return false;
	}
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];

	size_t bytesToRead;
	while (true)
	{
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







void MsgClient::execute(const size_t TimeBetweenMessages, const size_t NumMessages)
{
  ClientCounter counter;
  size_t myCount = counter.count();
  std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
  Show::attach(&std::cout);
  Show::start();
  Show::title(
    "Starting HttpMessage client" + myCountString + 
    " on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id())
  );
  try{
    SocketSystem ss;
    SocketConnecter si;
    while (!si.connect("localhost", 8080)) {
      Show::write("\n client waiting to connect");
      ::Sleep(100);
    }
    HttpMessage msg;
    for (size_t i = 0; i < NumMessages; ++i) {
      std::string msgBody = 
        "<msg>Message #" + Converter<size_t>::toString(i + 1) + 
        " from client #" + myCountString + "</msg>";
      msg = makeMessage(1, msgBody, "localhost:8080");
      sendMessage(msg, si);
      Show::write("\n\n  client" + myCountString + " sent\n" + msg.toIndentedString());
      ::Sleep(TimeBetweenMessages);
    }
    std::vector<std::string> files = FileSystem::Directory::getFiles("../TestFiles", "*.cpp");
    for (size_t i = 0; i < files.size(); ++i)
    {
      Show::write("\n\n  sending file " + files[i]);
      sendFile(files[i], si);
    }
    msg = makeMessage(1, "quit", "toAddr:localhost:8080");
    sendMessage(msg, si);
    Show::write("\n\n  client" + myCountString + " sent\n" + msg.toIndentedString());
    Show::write("\n");
    Show::write("\n  All done folks");
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}
//----< entry point - runs two clients each on its own thread >------

int main()
{
	::SetConsoleTitle(L"Clients Running on Thread");
	Show::title("Demostrating two HttpMessage Clients each running on a child thread");
	MsgClient c1;
	std::thread t1(
		[&]() {c1.execute(100, 20); }
	);
	MsgClient c2;
	std::thread t2(
		[&]() {c2.execute(120, 20); }
	);

	t1.join();
	t2.join();
}

