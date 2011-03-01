/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REMOTECONTROLHANDLER_H
#define REMOTECONTROLHANDLER_H

#include "handler.h"

#include <QString>

#include <gloox/iq.h>
#include <gloox/iqhandler.h>

namespace xrme {

class RemoteControlInterface;

class RemoteControlHandler : public Handler,
                             public gloox::IqHandler {
public:
  RemoteControlHandler(RemoteControlInterface* interface);

  void PlayPause(const QString& jid_resource);
  void Stop(const QString& jid_resource);
  void Next(const QString& jid_resource);
  void Previous(const QString& jid_resource);

  void QueryState(const QString& jid_resource);

  // Handler
  void Init(Connection* connection, gloox::Client* client);

  // gloox::IqHandler
  bool handleIq(const gloox::IQ& stanza);
  void handleIqID(const gloox::IQ&, int) {}

private:
  void SendIQ(const QString& jid_resource, gloox::IQ::IqType type,
              const QString& command);
  static int ParseInt(gloox::Tag* tag, const char* attribute_name);
  static double ParseDouble(gloox::Tag* tag, const char* attribute_name);
  static QString ParseString(gloox::Tag* tag, const char* attribute_name);

private:
  RemoteControlInterface* interface_;
};

} // namespace xrme

#endif // REMOTECONTROLHANDLER_H
