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
 * @file   timer.cpp
 * @author Mateusz Pusz
 * @date   Tue May  8 11:10:58 2007
 * 
 * @brief  
 * 
 * 
 */

#include "freettcn/te/timer.h"
#include "freettcn/te/behavior.h"
extern "C" {
#include <freettcn/ttcn3/tri_te_pa.h>
}
#include <iostream>


freettcn::TE::CTimer::CTimer():
  _defaultDurationValid(false)
{
}

freettcn::TE::CTimer::CTimer(TriTimerDuration defaultDuration) throw(EOperationFailed):
  _defaultDurationValid(true), _defaultDuration(defaultDuration), _command(0)
{
  if (_defaultDuration < 0) {
    std::cout << "ERROR: Timer default duration < 0!!!" << std::endl;
    throw freettcn::EOperationFailed();
  }
}

freettcn::TE::CTimer::~CTimer()
{
  if (_command)
    delete _command;
}
      
const TriTimerId &freettcn::TE::CTimer::Id() const
{
  return InstanceId();
}

void freettcn::TE::CTimer::Start(freettcn::TE::CTimer::CCommand *cmd) throw(freettcn::EOperationFailed)
{
  if (!_defaultDurationValid) {
    std::cout << "ERROR: Default duration not set!!!" << std::endl;
    throw freettcn::EOperationFailed();
  }
  
  if (!cmd) {
    std::cout << "ERROR: Timer command not set!!!" << std::endl;
    throw freettcn::EOperationFailed();
  }
  
  // set timer command
  _command = cmd;
  
  if (triStartTimer(&InstanceId(), _defaultDuration) == TRI_ERROR)
    throw freettcn::EOperationFailed();
}

void freettcn::TE::CTimer::Start(freettcn::TE::CTimer::CCommand *cmd, TriTimerDuration duration) throw(freettcn::EOperationFailed)
{
  if (duration < 0) {
    std::cout << "ERROR: Timer duration < 0!!!" << std::endl;
    throw freettcn::EOperationFailed();
  }
  
  if (!cmd) {
    std::cout << "ERROR: Timer command not set!!!" << std::endl;
    throw freettcn::EOperationFailed();
  }
  
  // set new timer command
  if (_command)
    delete _command;
  _command = cmd;
  
  if (triStartTimer(&InstanceId(), duration) == TRI_ERROR)
    throw freettcn::EOperationFailed();
}

void freettcn::TE::CTimer::Stop() throw(freettcn::EOperationFailed)
{
  delete _command;
  _command = 0;
  
  if (triStopTimer(&InstanceId()) == TRI_ERROR)
    throw freettcn::EOperationFailed();
}

TriTimerDuration freettcn::TE::CTimer::Read() const throw(freettcn::EOperationFailed)
{
  TriTimerDuration dur;
  if (triReadTimer(&InstanceId(), &dur) == TRI_ERROR)
    throw freettcn::EOperationFailed();
  return dur;
}

bool freettcn::TE::CTimer::Running() const throw(freettcn::EOperationFailed)
{
  unsigned char running;
  if (triTimerRunning(&InstanceId(), &running) == TRI_ERROR)
    throw freettcn::EOperationFailed();
  return running;
}

void freettcn::TE::CTimer::Timeout()
{
  _command->Run();
  
  delete _command;
  _command = 0;
}






freettcn::TE::CTimer::CCommand::~CCommand()
{
}

freettcn::TE::CTimer::CCmdBehaviorRun::CCmdBehaviorRun(const CBehavior &behavior):
  _behavior(behavior)
{
}

void freettcn::TE::CTimer::CCmdBehaviorRun::Run()
{
  _behavior.Run();
}