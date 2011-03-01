/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef BYTESTREAM_H__
#define BYTESTREAM_H__

#include "jid.h"
#include "logsink.h"

#include <string>

namespace gloox
{

  class BytestreamDataHandler;

  /**
   * @brief An abstraction of a single bytestream.
   *
   * Used as a base class for InBand Bytestreams as well as SOCKS5 Bytestreams.
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Bytestream
  {
    public:
      /**
       * Available stream types.
       */
      enum StreamType
      {
        S5B,                      /**< SOCKS5 Bytestream */
        IBB                       /**< In-Band Bytestream */
      };

      /**
       * Creates a new Bytestream.
       * @param type The stream type.
       * @param logInstance A Logsink to use for logging. Obtain it from ClientBase::logInstance().
       * @param initiator The initiator of the stream (usually the sender).
       * @param target The target of the stream (usually the receiver).
       * @param sid The stream's ID.
       */
      Bytestream( StreamType type, LogSink& logInstance, const JID& initiator, const JID& target,
                  const std::string& sid )
      : m_handler( 0 ), m_logInstance( logInstance ), m_initiator( initiator ), m_target( target ),
        m_type( type ), m_sid( sid ), m_open( false )
        {}

      /**
       * Virtual destructor.
       */
      virtual ~Bytestream() {}

      /**
       * Returns whether the bytestream is open, that is, accepted by both parties and ready
       * to send/receive data.
       * @return Whether or not the bytestream is open.
       */
      bool isOpen() const { return m_open; }

      /**
       * This function starts the connection process.
       * @return @b True if a connection to a remote entity could be established, @b false
       * otherwise.
       * @note If @b false is returned you should pass this Bytestream object
       * to SIProfileFT::dispose() for deletion.
       * @note Make sure you have a BytestreamDataHandler registered (using
       * registerBytestreamDataHandler()) before calling this function.
       */
      virtual bool connect() = 0;

      /**
       * Closes the bytestream.
       */
      virtual void close() = 0;

      /**
       * Use this function to send a chunk of data over an open bytestream.
       * If the stream is not open or has been closed again
       * (by the remote entity or locally), nothing is sent and @b false is returned.
       * This function does any base64 encoding for you, if necessary.
       * @param data The block of data to send.
       * @return @b True if the data has been sent (no guarantee of receipt), @b false
       * in case of an error.
       */
      virtual bool send( const std::string& data ) = 0;

      /**
       * Call this function repeatedly to receive data. You should even do this
       * if you use the bytestream to merely @b send data. May be a NOOP, depending on the actual
       * stream type.
       * @param timeout The timeout to use for select in microseconds. Default of -1 means blocking.
       * @return The state of the connection.
       */
      virtual ConnectionError recv( int timeout = -1 ) = 0;

      /**
       * Lets you retrieve the stream's ID.
       * @return The stream's ID.
       */
      const std::string& sid() const { return m_sid; }

      /**
       * Returns the stream's type.
       * @return The stream's type.
       */
      StreamType type() const { return m_type; }

      /**
       * Returns the target entity's JID. If this bytestream is remote-initiated, this is
       * the local JID. If it is local-initiated, this is the remote entity's JID.
       * @return The target's JID.
       */
      const JID& target() const { return m_target; }

      /**
       * Returns the initiating entity's JID. If this bytestream is remote-initiated, this is
       * the remote entity's JID. If it is local-initiated, this is the local JID.
       * @return The initiator's JID.
       */
      const JID& initiator() const { return m_initiator; }

      /**
       * Use this function to register an object that will receive any notifications from
       * the Bytestream instance. Only one BytestreamDataHandler can be registered
       * at any one time.
       * @param bdh The BytestreamDataHandler-derived object to receive notifications.
       */
      void registerBytestreamDataHandler( BytestreamDataHandler* bdh )
        { m_handler = bdh; }

      /**
       * Removes the registered BytestreamDataHandler.
       */
      void removeBytestreamDataHandler()
        { m_handler = 0; }

    protected:
      /** A handler for incoming data and open/close events. */
      BytestreamDataHandler* m_handler;

      /** A LogSink instance to use for logging. */
      const LogSink& m_logInstance;

      /** The initiator's JID. */
      const JID m_initiator;

      /** The target's JID. */
      const JID m_target;

      /** The stream type. */
      StreamType m_type;

      /** The stream ID. */
      std::string m_sid;

      /** Indicates whether or not the stream is open. */
      bool m_open;

    private:
      Bytestream& operator=( const Bytestream& );

  };

}

#endif // BYTESTREAM_H__
