/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "logsink.h"

namespace gloox
{

  LogSink::LogSink()
  {
  }

  LogSink::~LogSink()
  {
  }

  void LogSink::log( LogLevel level, LogArea area, const std::string& message ) const
  {
    LogHandlerMap::const_iterator it = m_logHandlers.begin();
    for( ; it != m_logHandlers.end(); ++it )
    {
      if( (*it).first && ( (*it).second.level <= level ) && ( (*it).second.areas & area ) )
        (*it).first->handleLog( level, area, message );
    }
  }

  void LogSink::registerLogHandler( LogLevel level, int areas, LogHandler* lh )
  {
    LogInfo info = { level, areas };
    m_logHandlers[lh] = info;
  }

  void LogSink::removeLogHandler( LogHandler* lh )
  {
    m_logHandlers.erase( lh );
  }

}
