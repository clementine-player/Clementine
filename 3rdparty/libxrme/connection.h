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

#ifndef LIBXRME_CONNECTION_H
#define LIBXRME_CONNECTION_H

#include <QObject>
#include <QPair>
#include <QScopedPointer>
#include <QString>

namespace xrme {

class MediaPlayerInterface;
class RemoteControlInterface;

class Connection : public QObject {
  Q_OBJECT

public:
  Connection(QObject* parent = NULL);
  ~Connection();

  struct Peer {
    Peer();

    enum Capability {
      None = 0x00,

      MediaPlayer = 0x01,
      RemoteControl = 0x02,
      MediaStorage = 0x04,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability);

    QString agent_name_;
    QString jid_resource_;
    Capabilities caps_;
  };
  typedef QList<Peer> PeerList;

  // The username and password MUST be set before calling Connect().
  void set_username(const QString& username);
  void set_password(const QString& password);

  // The agent name MUST be set before calling Connect().  The agent name is a
  // friendly user-visible name assigned to this agent to help the user pick it
  // out amongst other media players or remotes.  It might contain the name of
  // the application or the computer's hostname.
  void set_agent_name(const QString& agent_name);

  // Sets the hostname of the XMPP server to connect to.  Defaults to
  // "talk.google.com".
  void set_server(const QString& server);

  // Sets the resource string to use in the JID.  Defaults to "xrmeagent".
  void set_jid_resource(const QString& resource);

  // Sets the host part to append to the JID if the user didn't specify one in
  // the username.  The host part is the part of the JID after the @ and before
  // the /, eg: "username@host/resource".  Defaults to "gmail.com".
  void set_jid_host(const QString& host);

  // If this is set then detailed XMPP output will be printed.  Defaults to off.
  void set_verbose(bool verbose);

  // The following getters just return the data set above.
  QString username() const;
  QString password() const;
  QString agent_name() const;
  QString server() const;
  QString jid_resource() const;
  QString jid_host() const;
  bool is_verbose() const;

  // Sets a media player on the connection.  Calling this means the XRME agent
  // will advertise itself as a media player, and will be able to controlled by
  // remote control agents.  Should be called before calling Connect().  The
  // Connection will NOT take ownership of the MediaPlayerInterface, and the
  // MediaPlayerInterface MUST stay alive for as long as the connection.
  void SetMediaPlayer(MediaPlayerInterface* iface);

  // Sets a remote control on the connection.  Calling this means the XRME agent
  // will advertise itself as a remote control, and will receive state changes
  // from media player agents.  Should be called before calling Connect().  The
  // Connection will NOT take ownership of the RemoteControlInterface, and the
  // RemoteControlInterface MUST stay alive for as long as the connection.
  void SetRemoteControl(RemoteControlInterface* iface);

  // Returns true after Connected() is emitted.
  bool is_connected() const;

  // Returns the user's actual JID.  This is only valid if is_connected() is
  // true.  Before the connection is complete it will return a null QString.
  QString jid() const;

  // Returns the list of all known peers.  You can refresh this list by calling
  // RefreshPeers().  PeerFound() is emitted any time a peer is added to this
  // list.
  PeerList peers() const;

  // Returns the list of known peers that support a certain capability.
  PeerList peers(Peer::Capability cap) const;

public slots:
  // Starts connecting and returns immediately.  Will emit Connected() or
  // Disconnected() later.  The username and password must already be set.
  // Returns true on success, false if there was some problem starting the
  // connection (eg. invalid/unspecified username/password/agent_name).
  bool Connect();

  // Disconnects immediately and emits Disconnected().
  void Disconnect();

  // Clears the internal list of peers and sends XMPP queries to discover the
  // current list.  Emits PeerFound().
  void RefreshPeers();

signals:
  void Connected();
  void Disconnected(const QString& error);

  void PeerFound(const xrme::Connection::Peer& peer);
  void PeerRemoved(const xrme::Connection::Peer& peer);

private slots:
  void SocketReadyReceive();
  void CleanupClient();

private:
  struct Private;
  friend struct Private;
  QScopedPointer<Private> d;
};

} // namespace xrme

#endif
