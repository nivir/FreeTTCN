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
 * @file   timer.h
 * @author Mateusz Pusz
 * @date   Wed Apr 25 10:59:31 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <freettcn/te/idObject.h>
extern "C" {
#include <freettcn/ttcn3/tri.h>
}
#include <freettcn/tools/exception.h>


namespace freettcn {
  
  namespace TE {
    
//     enum TVerdictType_t {
//       VERDICT_NONE,
//       VERDICT_PASS,
//       VERDICT_INCONC,
//       VERDICT_FAIL,
//       VERDICT_ERROR
//     };
    
    class CBehavior;
    
    class CTimer : public CIdObject {
    public:
      class CCommand {
      public:
        virtual ~CCommand();
        virtual void Run() = 0;
      };
      
      class CCmdBehaviorRun : public CCommand {
        const CBehavior &_behavior;
      public:
        CCmdBehaviorRun(const CBehavior &behavior);
        virtual void Run();
      };
      
    private:
      class CState {
        // STATUS:
        //  - IDLE
        //  - RUNNING
        //  - TIMEOUT
        // DEF_DURATION - default duration of a timer (undefined if not inited during declaration)
        // ACT_DURATION - stores the actual duration with which a running timer has been started (0.0 - if timer is stopped or timed out) (if a timer is started without duration, DEF_DURATION is copied) - error if no value defined when starting;
        // TIME_LEFT - describes the actual duration that a running timer has to run before it times out (0.0 - if timer is stopped or timed out)
        // SNAP_VALUE - when taking a snapshot it gets the actual value of ACT_DURATION - TIME_LEFT
        // SNAP_STATUS - when taking a snapshot it gets the same value as STATUS
      };
      
      const bool _defaultDurationValid;
      TriTimerDuration _defaultDuration;
      CCommand *_command;
      
    public:
      CTimer();
      CTimer(TriTimerDuration defaultDuration) throw(EOperationFailed);
      ~CTimer();
      
      const TriTimerId &Id() const;
      
      void Start(CCommand *cmd) throw(EOperationFailed);
      void Start(CCommand *cmd, TriTimerDuration duration) throw(EOperationFailed);
      void Stop() throw(EOperationFailed);
      TriTimerDuration Read() const throw(EOperationFailed);
      bool Running() const throw(EOperationFailed);
      
      void Timeout();
    };
    
  } // namespace TE
  
} // namespace freettcn


#endif /* __TIMER_H__ */
