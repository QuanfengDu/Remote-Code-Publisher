#ifndef MOCKCHANNEL_H
#define MOCKCHANNEL_H
///////////////////////////////////////////////////////////////////////
// MockChannel.pp                                                    //
// ver 3.9                                                           //
// Language:    C++, Visual Studio 2015                              //
// Platform:    Dell XPS 13, Windows 10                              //
// Application:  Object Oriented Design                              //
// Author:      Quanfeng Du, Syracuse University,                    //
//              qdu101@syr.edu                                       //
///////////////////////////////////////////////////////////////////////

#ifdef IN_DLL
#define DLL_DECL __declspec(dllexport)
#else
#define DLL_DECL __declspec(dllimport)
#endif

#include <string>
#include"../HttpMessage/HttpMessage.h"

using Message = HttpMessage;

struct ISendr
{
  virtual void postMessage(const Message& msg) = 0;
};

struct IRecvr
{
  virtual Message getMessage() = 0;
};

struct IMockChannel
{
public:
  virtual void start() = 0;
  virtual void stop() = 0;
};

extern "C" {
  struct ObjectFactory
  {
    DLL_DECL ISendr* createSendr();
    DLL_DECL IRecvr* createRecvr();
    DLL_DECL IMockChannel* createMockChannel(ISendr* pISendr, IRecvr* pIRecvr);
  };
}

#endif


