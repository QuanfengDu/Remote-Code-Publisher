#pragma once
///////////////////////////////////////////////////////////////////////
// MsgServer.cpp                                                    //
// ver 3.9                                                           //
// Language:    C++, Visual Studio 2015                              //
// Platform:    Dell XPS 13, Windows 10                              //
// Application:  Object Oriented Design                              //
// Author:      Quanfeng Du, Syracuse University,                    //
//              qdu101@syr.edu                                       //
///////////////////////////////////////////////////////////////////////
/*
* This package implements a server that receives HTTP style messages and
* files from multiple concurrent clients and simply displays the messages
* and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/
/*
* Required Files:
*   MsgClient.cpp, MsgServer.cpp
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp
*   Utilities.h, Utilities.cpp
*/
/*
* ToDo:
* - pull the receiving code into a Receiver class
* - Receiver should own a BlockingQueue, exposed through a
*   public method:
*     HttpMessage msg = Receiver.GetMessage()
* - You will start the Receiver instance like this:
*     Receiver rcvr("localhost:8080");
*     ClientHandler ch;
*     rcvr.start(ch);
*/
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>

using Show = StaticLogger<1>;
using namespace Utilities;
using EndPoint = std::string;
/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You no longer need to be careful using data members of this class
//   because each client handler thread gets its own copy of this 
//   instance so you won't get unwanted sharing.
// - I changed the SocketListener semantics to pass
//   instances of this class by value for version 5.2.
// - that means that all ClientHandlers need copy semantics.
//
class ClientHandler
{
public:
	ClientHandler(BlockingQueue<HttpMessage>& msgQ, BlockingQueue<HttpMessage>& msgQSend) : msgQ_(msgQ),msgQ_Send(msgQSend) {}
	void operator()(Socket socket);
	HttpMessage makeMessage(size_t n, const std::string& body, const EndPoint& ep);
	bool sendFile(const std::string& fqname, Socket& socket);
	void viewallhtml(Socket &socket);
	void sendMessage(HttpMessage& msg, Socket& socket);
	void setdownload(HttpMessage temp,Socket& socket);
	void setdelete(HttpMessage temp);
	void setcategory(Socket &socket);
	void setdetails(HttpMessage temp, Socket &socket);
	void setconnect(Socket &socket);


private:
	bool connectionClosed_;
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
	BlockingQueue<HttpMessage>& msgQ_;
	BlockingQueue<HttpMessage>& msgQ_Send;
	
};