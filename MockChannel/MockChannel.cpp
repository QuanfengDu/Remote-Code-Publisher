///////////////////////////////////////////////////////////////////////
// MockChannel.pp                                                    //
// ver 3.9                                                           //
// Language:    C++, Visual Studio 2015                              //
// Platform:    Dell XPS 13, Windows 10                              //
// Application:  Object Oriented Design                              //
// Author:      Quanfeng Du, Syracuse University,                    //
//              qdu101@syr.edu                                       //
///////////////////////////////////////////////////////////////////////

#define IN_DLL
#include "MockChannel.h"
#include "Cpp11-BlockingQueue.h"
#include <string>
#include <thread>
#include <iostream>
#include"../MsgClient/MsgClient.h"
#include<string>;

using BQueue = BlockingQueue < Message >;
using BQueueString = BlockingQueue<std::string>;

/////////////////////////////////////////////////////////////////////////////
// Sendr class
// - accepts messages from client for consumption by MockChannel
//
class Sendr : public ISendr
{
public:
  void postMessage(const Message& msg);
  BQueue& queue();
private:
  BQueue sendQ_;
};

void Sendr::postMessage(const Message& msg)
{
  sendQ_.enQ(msg);
}

BQueue& Sendr::queue() { return sendQ_; }

/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
class Recvr : public IRecvr
{
public:
  Message getMessage();
  BQueue& queue();
private:
	BQueue recvQ_;
};

Message Recvr::getMessage()
{
  return recvQ_.deQ();
}

BQueue& Recvr::queue()
{
  return recvQ_;
}
/////////////////////////////////////////////////////////////////////////////
// MockChannel class
// - reads messages from Sendr and writes messages to Recvr
//
class MockChannel : public IMockChannel
{
public:
  MockChannel(ISendr* pSendr, IRecvr* pRecvr);
  void start();
  void stop();
  /*void sendMessage(Message);*/
private:
  std::thread thread_;
  ISendr* pISendr_;
  IRecvr* pIRecvr_;
  bool stop_ = false;
};

//----< pass pointers to Sender and Receiver >-------------------------------

MockChannel::MockChannel(ISendr* pSendr, IRecvr* pRecvr) : pISendr_(pSendr), pIRecvr_(pRecvr) {}

//----< creates thread to read from sendQ and echo back to the recvQ >-------

void MockChannel::start()
{
  thread_ = std::thread(
    [this] {
    Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
    Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);
    if (pSendr == nullptr || pRecvr == nullptr){
      std::cout << "\n  failed to start Mock Channel\n\n";
      return;
    }
    BQueue& sendQ = pSendr->queue();
	BQueue& recvQ = pRecvr->queue();
	MsgClient client;
	while (!stop_){
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080)){
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}
		std::thread t2(
			[&]() {
			while (true) {
				HttpMessage http = client.readMessage(si);
				std::string str = http.bodyString();
				if (!str.size())
					continue;
				std::cout << "\n\n Client Received HttpMessage: \n" << http.toString();
				recvQ.enQ(http);
			}});
		t2.detach();
		std::thread t1(
			[&]() {
			while (!stop_)
			{
				Message msg = sendQ.deQ();  // will block here so send quit message when stopping
				std::string tt = msg.findValue("command");
				BlockingQueue<HttpMessage> msgQ;
				if (tt != "")
				{
					std::cout << "\n Client send HttpMessage: \n" << msg.toString();
					MsgClient MC;
					MC.sendMessage(msg, si);
					std::string file_ = msg.findValue("file");
					if (file_ != "")
					{
						MC.sendFile(file_, si);
					}}}});
		t1.join();}});
}

//----< signal server thread to stop >---------------------------------------

void MockChannel::stop() { stop_ = true; }

//----< factory functions >--------------------------------------------------

ISendr* ObjectFactory::createSendr() { return new Sendr; }

IRecvr* ObjectFactory::createRecvr() { return new Recvr; }

IMockChannel* ObjectFactory::createMockChannel(ISendr* pISendr, IRecvr* pIRecvr) 
{ 
  return new MockChannel(pISendr, pIRecvr); 
}

#ifdef TEST_MOCKCHANNEL

//----< test stub >----------------------------------------------------------

int main()
{
  ObjectFactory objFact;
  ISendr* pSendr = objFact.createSendr();
  IRecvr* pRecvr = objFact.createRecvr();
  IMockChannel* pMockChannel = objFact.createMockChannel(pSendr, pRecvr);
  pMockChannel->start();
  pSendr->postMessage("Hello World");
  pSendr->postMessage("CSE687 - Object Oriented Design");
  Message msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pSendr->postMessage("stopping");
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pMockChannel->stop();
  pSendr->postMessage("quit");
  std::cin.get();
}
#endif
