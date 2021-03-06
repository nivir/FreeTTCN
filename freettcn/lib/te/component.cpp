//
// Copyright (C) 2007 Mateusz Pusz
//
// This file is part of freettcn (Free TTCN) library.

// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/**
 * @file   component.cpp
 * @author Mateusz Pusz
 * @date   Wed Apr 25 13:38:56 2007
 * 
 * @brief  
 * 
 * 
 */

#include "freettcn/te/component.h"
#include "freettcn/te/te.h"
#include "freettcn/te/module.h"
#include "freettcn/te/behavior.h"
#include "freettcn/te/timer.h"
#include "freettcn/te/testCase.h"
#include "freettcn/te/port.h"
#include "freettcn/te/basicTypes.h"
#include "freettcn/te/sourceData.h"
#include "freettcn/tools/logMask.h"
#include "freettcn/tools/timeStampMgr.h"
#include "freettcn/tools/tools.h"
#include "freettcn/ttcn3/tci_te_ch.h"
#include "freettcn/ttcn3/tci_tl.h"
#include <cstring>
#include <iostream>



/* ********************************* I N S T A N C E ************************************ */

freettcn::TE::CTestComponentType::CInstance::CInstance(const CType &type):
  freettcn::TE::CType::CInstance(type)
{
}

freettcn::TE::CTestComponentType::CInstance::~CInstance()
{
  Purge(_portIdArray);
}

const TriPortId &freettcn::TE::CTestComponentType::CInstance::PortId(const char *name, int idx) const
{
  for(unsigned int i=0; i<_portIdArray.size(); i++) {
    if (!strcmp(name, _portIdArray[i]->portName)) {
      if (!idx && idx == _portIdArray[i]->portIndex)
        return *_portIdArray[i];
      else {
        // skip to correct port index
        if (i + idx < _portIdArray.size()) {
          /// @todo may be removed in optimizations
          // check if correct name and index
          if (!strcmp(name, _portIdArray[i + idx]->portName) &&
              idx == _portIdArray[i + idx]->portIndex)
            return *_portIdArray[i + idx];
        }
      }
      break;
    }
  }
  
  throw ENotFound(E_DATA, "Port ID not found!!!");
}



/* *************************** I N S T A N C E   R E M O T E **************************** */

freettcn::TE::CTestComponentType::CInstanceRemote::CInstanceRemote(const CType &type, const TriComponentId &id, TciTestComponentKindType kind):
  CInstance(type), _id(id), _kind(kind), _status(IDLE)
{
}

const TriComponentId &freettcn::TE::CTestComponentType::CInstanceRemote::Id() const
{
  return _id.Id();
}

TciTestComponentKindType freettcn::TE::CTestComponentType::CInstanceRemote::Kind() const
{
  return _kind;
}

void freettcn::TE::CTestComponentType::CInstanceRemote::Start(const CBehavior &behavior, TciParameterListType parameterList)
{
  tciStartTestComponentReq(_id.Id(), behavior.Id(), parameterList);
  _status = ACTIVE;
}

void freettcn::TE::CTestComponentType::CInstanceRemote::Stop()
{
  tciStopTestComponentReq(_id.Id());
  if (_kind != TCI_ALIVE_COMP)
    std::cout << "WARNING: Only ALIVE components should be stopped (use 'kill') instead." << std::endl; 
  _status = TERMINATED;
}

void freettcn::TE::CTestComponentType::CInstanceRemote::Kill()
{
  _status = KILLED;
  tciKillTestComponentReq(_id.Id());
}

bool freettcn::TE::CTestComponentType::CInstanceRemote::operator==(const TriComponentId &id) const
{
  const TriComponentId &localId = _id.Id();
  
  return (id.compInst.bits == localId.compInst.bits) &&
    !memcmp(id.compInst.data, localId.compInst.data, id.compInst.bits / 8);
}

freettcn::TE::CTestComponentType::CInstanceRemote::TStatus freettcn::TE::CTestComponentType::CInstanceRemote::Status() const
{
  return _status;
}




/* **************************** I N S T A N C E   L O C A L ***************************** */

freettcn::TE::CTestComponentType::CInstanceLocal::CInstanceLocal(const CType &type):
  freettcn::TE::CTestComponentType::CInstance(type), _module(0), _startTimer(0),
  _status(NOT_INITED), _verdict(CBasicTypes::Verdict(), VERDICT_NONE), _guardTimer(0),
  _behavior(0), _scope(0), _behaviorOffset(CBehavior::OFFSET_AUTO)
{
  
}


freettcn::TE::CTestComponentType::CInstanceLocal::~CInstanceLocal()
{
  Purge(_portArray);

  Reset();
  
  // purge component stack
  ScopePop();
}


const TriComponentId &freettcn::TE::CTestComponentType::CInstanceLocal::Id() const
{
  if (_status == NOT_INITED)
    throw ENotInitialized(E_DATA, "Status not inited!!!");
  return _id;
}


TciTestComponentKindType freettcn::TE::CTestComponentType::CInstanceLocal::Kind() const
{
  if (_status == NOT_INITED)
    throw ENotInitialized(E_DATA, "Status not inited!!!");
  return _kind;
}


void freettcn::TE::CTestComponentType::CInstanceLocal::Start(const CBehavior &behavior, TciParameterListType parameterList)
{
  if (_status == NOT_INITED)
    throw ENotInitialized(E_DATA, "Status not inited");
  
  _behavior = &behavior;
  
  // schedule executing test component
  _startTimer = new freettcn::TE::CTimer(*this, true, 0);
  _startTimer->Start();
  _startTimer->HandlerAdd(CBehavior::OFFSET_START);
  
  _status = ACTIVE;
}


void freettcn::TE::CTestComponentType::CInstanceLocal::Stop()
{
  freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
  if (te.Logging() && te.LogMask().Get(freettcn::CLogMask::CMD_TE_C_TERMINATED))
    // log
    tliCTerminated(0, te.TimeStampMgr().Get(), 0, 0, Id(), &_verdict);
  
  tciTestComponentTerminatedReq(Id(), &_verdict);
  
  Reset();
  
  _status = BLOCKED;
}


void freettcn::TE::CTestComponentType::CInstanceLocal::Kill()
{
  if (_kind == TCI_MTC_COMP) {
    // kill all active PTCs
    _module->TestComponentAllKill(0, 0, *this);
    
    // kill SYSTEM component
    _module->ActiveTestCase().System().Kill();
  }

  freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
  if (te.Logging() && te.LogMask().Get(freettcn::CLogMask::CMD_TE_C_TERMINATED))
    // log
    tliCTerminated(0, te.TimeStampMgr().Get(), 0, 0, Id(), &_verdict);
  
  tciTestComponentTerminatedReq(Id(), &_verdict);
  
  // add to killed list
  /// @todo add to killed list (only local?)
  
  delete this;
}






void freettcn::TE::CTestComponentType::CInstanceLocal::Reset()
{
  if (_startTimer) {
    delete _startTimer;
    _startTimer = 0;
  }
  if (_guardTimer) {
    delete _guardTimer;
    _guardTimer = 0;
  }
  
  // reset component scope to the first level
  while(_scope->Up())
    ScopePop();
}


freettcn::TE::CModule &freettcn::TE::CTestComponentType::CInstanceLocal::Module() const
{
  if (_status == NOT_INITED)
    throw ENotInitialized(E_DATA, "Status not inited!!!");
  
  return *_module;
}


freettcn::TE::CTestComponentType::CInstanceLocal::TStatus freettcn::TE::CTestComponentType::CInstanceLocal::Status() const
{
  return _status;
}

void freettcn::TE::CTestComponentType::CInstanceLocal::Status(TStatus status)
{
  if (status == ACTIVE && _guardTimer) {
    delete _guardTimer;
    _guardTimer = 0;
  }
  
  _status = status;
}

void freettcn::TE::CTestComponentType::CInstanceLocal::Init(CModule &module, TciTestComponentKindType kind, const char *name)
{
  _module = &module;
  _kind = kind;
  
  _id.compInst = InstanceId();
  _id.compName = const_cast<char *>(name);
  _id.compType = Type().Id();
  
  _status = BLOCKED;
  
  // create ports
  const CTestComponentType &type = static_cast<const CTestComponentType &>(Type());
  for(unsigned int i=0; i<type.PortInfoNum(); i++) {
    CPort *port = new CPort(type.PortInfo(i), *this);
    _portArray.push_back(port);
    
    /// @todo Maybe it should be added after 'Start' operation
    /// @todo PortRemove should be added
    _module->ActiveTestCase().PortAdd(*port);
  }
  
  // perform component specific initialization
  freettcn::TE::CInitObject::Init();
  
  ScopePush("component");
  /// @todo init port scope
}


void freettcn::TE::CTestComponentType::CInstanceLocal::Execute(const char *src, int line,
                                                               CTestCase &testCase, TriTimerDuration duration,
                                                               int returnOffset)
{
  // create and start MTC
  testCase.Start(const_cast<char *>(src), line, this, 0, duration);
  
  // block Control component for the time of testcase execution
  if (duration) {
    _status = SNAPSHOT;
    
    // set test case guard timer
    _guardTimer = new CTimer(*this, true, duration);
    _guardTimer->Start();
    _guardTimer->HandlerAdd(CBehavior::GUARD_TIMEOUT);
  }
  else {
    _status = BLOCKED;
  }
  
  // set offset to be run when component terminates
  _behaviorOffset = returnOffset;
}


// void freettcn::TE::CTestComponentType::CInstanceLocal::Run()
// {
//   if (_status == BLOCKED)
//     // blocked components do not run commands
//     return;
  
//   // get top command from the stack
//   while(CCommand *cmd = _controlStack.First())
//     // run the command
//     if (cmd->Run()) {
//       // next command should be run - dequeue finished command
//       _controlStack.Dequeue();
      
//       if (_status == BLOCKED)
//         // blocked components do not run commands
//         return;
//     }
//     else
//       // command did not finish
//       return;
  
//   // test component done
//   if (_kind != TCI_ALIVE_COMP)
//     delete this;
// }


freettcn::TE::CPort &freettcn::TE::CTestComponentType::CInstanceLocal::Port(const char *name, int idx) const
{
  for(unsigned int i=0; i<_portArray.size(); i++) {
    const TriPortId *portId = &_portArray[i]->Id();
    if (!strcmp(name, portId->portName)) {
      if (!idx && idx == portId->portIndex)
        return *_portArray[i];
      else {
        // skip to correct port index
        if (i + idx < _portArray.size()) {
          /// @todo may be removed in optimizations
          // check if correct name and index
          portId = &_portArray[i + idx]->Id();
          if (!strcmp(name, portId->portName) &&
              idx == portId->portIndex)
            return *_portArray[i + idx];
        }
      }
      break;
    }
  }
  
  throw ENotFound(E_DATA, "Port not found!!!");
}
        

void freettcn::TE::CTestComponentType::CInstanceLocal::Run(unsigned int offset)
{
  if (!_behavior)
    throw EOperationFailed(E_DATA, "Behavior not started!!!");
  
  if (offset == CBehavior::GUARD_TIMEOUT) {
    // kill MTC
    _module->ActiveTestCase().Stop();
    return;
  }
  
  if (offset != CBehavior::OFFSET_AUTO)
    _behaviorOffset = offset;
  
  unsigned int next = CBehavior::ERROR;
  do {
    next = _behavior->Run(*this, _behaviorOffset);
    switch(next) {
    case CBehavior::ERROR:
    case CBehavior::OFFSET_START:
      std::cout << "ERROR: Behavior running error!!!" << std::endl;
      return;
      
    case CBehavior::END:
      // test component done
      return;
      
    case CBehavior::WAIT:
      // _behaviorOffset already set - do not overwrite
      return;
      
    default:
      _behaviorOffset = next;
    }
  }
  while(next != CBehavior::WAIT);
}


void freettcn::TE::CTestComponentType::CInstanceLocal::StopReq(const char *src, int line, CInstanceRemote *comp /* 0 */)
{
//   freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
//   if (te.Logging() && te.LogMask().Get(freettcn::CLogMask::CMD_TE_C_STOP))
//     // log
//     tliCStop(0, te.TimeStampMgr().Get(), src, line, Id(), &_verdict);
  
  if (!comp) {
    // self
    if (_kind == TCI_ALIVE_COMP)
      Stop();
    else
      Kill();
  }
  else {
    // other test component
    if (comp->Kind() == TCI_ALIVE_COMP) {
      if (comp->Status() != CInstanceRemote::TERMINATED)
        comp->Stop();
    }
    else {
      if (comp->Status() != CInstanceRemote::KILLED)
        comp->Kill();
    }
  }
}


void freettcn::TE::CTestComponentType::CInstanceLocal::StopAllReq(const char *src, int line)
{
  if (_kind != TCI_MTC_COMP)
    throw EOperationFailed(E_DATA, "Only MTC component can stop ALL running test comopnents!!!");
  
  _module->TestComponentAllStop(src, line, *this);
}


void freettcn::TE::CTestComponentType::CInstanceLocal::TimerAdd(const CTimer &timer, bool implicit /* false */)
{
  CTimerList &list = implicit ? _implicitTimers : _explicitTimers;
  list.push_back(&timer);
}


void freettcn::TE::CTestComponentType::CInstanceLocal::TimerRemove(const CTimer &timer, bool implicit /* false */)
{
  CTimerList &list = implicit ? _implicitTimers : _explicitTimers;
  
  for(CTimerList::iterator it=list.begin(); it!=list.end(); ++it) {
    if (*it == &timer) {
      list.erase(it);
      return;
    }
  }
  
  throw ENotFound(E_DATA, "Timer not found!!!");
}


// void freettcn::TE::CTestComponentType::CInstanceLocal::Map(const freettcn::TE::CPort &fromPort, const freettcn::TE::CPort &toPort)
// {
//   if (_status == NOT_INITED)
//     throwfreettcn::TE::CTestComponentType::CInstanceLocal::ENotInitialized();
// }


void freettcn::TE::CTestComponentType::CInstanceLocal::Verdict(const char *src, int line, TVerdict verdict)
{
  freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
  if (te.Logging() && te.LogMask().Get(freettcn::CLogMask::CMD_TE_C_TERMINATED)) {
    // log
    CVerdictType::CInstance tciVerdict(CBasicTypes::Verdict(), verdict);
    tliSetVerdict(0, te.TimeStampMgr().Get(), const_cast<char *>(src), line, Id(), &tciVerdict);
  }
  
  // update verdict
  if (verdict > _verdict.Value())
    _verdict.Value(verdict);
}


// void freettcn::TE::CTestComponentType::CInstanceLocal::Enqueue(CCommand *cmd)
// {
//   _controlStack.Enqueue(cmd);
// }



void freettcn::TE::CTestComponentType::CInstanceLocal::ScopePush(const char *kind)
{
  _scope = new CScope(kind, _scope);
}


void freettcn::TE::CTestComponentType::CInstanceLocal::ScopePop()
{
  CScope *scope = _scope->Up();
  delete _scope;
  _scope = scope;
}


void freettcn::TE::CTestComponentType::CInstanceLocal::ScopeEnter(const char *src, int line, const char *kind)
{
  ScopePush(kind);

  freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
  if (te.Logging() && te.LogMask().Get(freettcn::CLogMask::CMD_TE_S_ENTER)) {
    // log
    /// @todo parameters dump
    TciParameterListType parList;
    parList.length = 0;
    parList.parList = 0;

    /// @todo scope name
    QualifiedName name = { 0, 0, 0 };
    
    tliSEnter(0, te.TimeStampMgr().Get(), const_cast<char *>(src), line, Id(), name, parList, const_cast<char *>(kind));
  }
}


freettcn::TE::CTestComponentType::CInstanceLocal::CScope &freettcn::TE::CTestComponentType::CInstanceLocal::Scope() const
{
  if (_scope)
    return *_scope;
  
  throw ENotFound(E_DATA, "Scope not found!!!");
}


void freettcn::TE::CTestComponentType::CInstanceLocal::ScopeLeave(const char *src, int line)
{
  if (_scope) {
    freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
    if (te.Logging() && te.LogMask().Get(freettcn::CLogMask::CMD_TE_S_ENTER)) {
      // log
      /// @todo parameters dump
      TciParameterListType parList;
      parList.length = 0;
      parList.parList = 0;
      
      /// @todo scope name
      QualifiedName name = { 0, 0, 0 };
      
      /// @todo return value
      
      tliSLeave(0, te.TimeStampMgr().Get(), const_cast<char *>(src), line, Id(), name, parList, 0, const_cast<char *>(_scope->Kind()));
    }
    
    ScopePop();
  }
  else
    throw EOperationFailed(E_DATA, "Already on the top scope!!!");
}



void freettcn::TE::CTestComponentType::CInstanceLocal::ConnectReq(const TriPortId &port1, const TriPortId &port2)
{
  tciConnectReq(port1, port2);
}

void freettcn::TE::CTestComponentType::CInstanceLocal::DisconnectReq(const TriPortId &port1, const TriPortId &port2)
{
  tciDisconnectReq(port1, port2);
}

void freettcn::TE::CTestComponentType::CInstanceLocal::MapReq(const TriPortId &port1, const TriPortId &port2)
{
  tciMapReq(port1, port2);
}

void freettcn::TE::CTestComponentType::CInstanceLocal::UnmapReq(const TriPortId &port1, const TriPortId &port2)
{
  tciUnmapReq(port1, port2);
}









freettcn::TE::CTestComponentType::CInstanceLocal::CScope::CScope(const char *kind, CScope *up):
  _kind(kind), _up(up)
{
}


freettcn::TE::CTestComponentType::CInstanceLocal::CScope::~CScope()
{
  Purge(_valueArray);
  Purge(_timerArray);
}


const char *freettcn::TE::CTestComponentType::CInstanceLocal::CScope::Kind() const
{
  return _kind;
}


freettcn::TE::CTestComponentType::CInstanceLocal::CScope *freettcn::TE::CTestComponentType::CInstanceLocal::CScope::Up() const
{
  return _up;
}


void freettcn::TE::CTestComponentType::CInstanceLocal::CScope::Register(CType::CInstance *value)
{
  _valueArray.push_back(value);
}


freettcn::TE::CType::CInstance &freettcn::TE::CTestComponentType::CInstanceLocal::CScope::Value(unsigned int valueIdx) const
{
  if (valueIdx < _valueArray.size())
    return *_valueArray[valueIdx];
  
  std::stringstream stream;
  stream << "Value index: " << valueIdx << " too big (size: " << _valueArray.size() << ")";
  throw EOutOfRange(E_DATA, stream.str());
}


void freettcn::TE::CTestComponentType::CInstanceLocal::CScope::Register(CTimer *timer)
{
  _timerArray.push_back(timer);
}


freettcn::TE::CTimer &freettcn::TE::CTestComponentType::CInstanceLocal::CScope::Timer(unsigned int timerIdx) const
{
  if (timerIdx < _timerArray.size())
    return *_timerArray[timerIdx];
  
  std::stringstream stream;
  stream << "Timer index: " << timerIdx << " too big (size: " << _timerArray.size() << ")";
  throw EOutOfRange(E_DATA, stream.str());
}




/* ************************* T E S T   C O M P O N E N T   T Y P E ********************** */

freettcn::TE::CTestComponentType::CTestComponentType(const freettcn::TE::CModule *module, const char *name) :
  freettcn::TE::CType(module, name, TCI_COMPONENT_TYPE, nullptr, nullptr, nullptr)
{
  _portList.length = 0;
  _portList.portIdList = 0;
}


freettcn::TE::CTestComponentType::~CTestComponentType()
{
  Purge(_portInfoArray);
  
  for(int i=0; i<_portList.length; i++)
    delete _portList.portIdList[i];
  delete[] _portList.portIdList;
}


void freettcn::TE::CTestComponentType::Register(std::shared_ptr<CTriPortId> id)
{
  CPortInfo *portInfo = new CPortInfo(portType, name, portIdx);
  _portInfoArray.push_back(portInfo);
  
  
//   _portIdList.push_back(portId);
}


void freettcn::TE::CTestComponentType::Init()
{
  _portList.length = _portInfoArray.size();
  if (_portList.length) {
    _portList.portIdList = new TriPortId *[_portList.length];

    for(unsigned int i=0; i<_portInfoArray.size(); i++) {
      _portList.portIdList[i] = new TriPortId;
      
      memset(&_portList.portIdList[i]->compInst, 0, sizeof(TriComponentId)); /**< component not known for now */
      _portList.portIdList[i]->portName = const_cast<char *>(_portInfoArray[i]->Name());
      _portList.portIdList[i]->portIndex = _portInfoArray[i]->PortIdx();
      _portList.portIdList[i]->portType = _portInfoArray[i]->Type().Id();
      _portList.portIdList[i]->aux = 0;
    }
  }
}


TriPortIdList freettcn::TE::CTestComponentType::Ports() const
{
  return _portList;
}


unsigned int freettcn::TE::CTestComponentType::PortInfoNum() const
{
  return _portInfoArray.size();
}


const freettcn::TE::CPortInfo &freettcn::TE::CTestComponentType::PortInfo(unsigned int idx) const
{
  if (idx < _portInfoArray.size())
    return *_portInfoArray[idx];
  
  std::stringstream stream;
  stream << "Port info index: " << idx << " too big (size: " << _portInfoArray.size() << ")";
  throw EOutOfRange(E_DATA, stream.str());
}





freettcn::TE::CControlComponentType::CControlComponentType(const std::shared_ptr<const TciModuleId> &moduleId):
  freettcn::TE::CTestComponentType(0 , "_ControlComponentType_")
{
}

freettcn::TE::CControlComponentType::CInstance *freettcn::TE::CControlComponentType::InstanceCreate() const
{
  return new freettcn::TE::CControlComponentType::CInstance(*this);
}



freettcn::TE::CControlComponentType::CInstance::CInstance(const freettcn::TE::CType &type):
  freettcn::TE::CTestComponentType::CInstanceLocal(type)
{
}

void freettcn::TE::CControlComponentType::CInstance::Initialize()
{
}


