/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef INBANDBYTESTREAM_H__
#define INBANDBYTESTREAM_H__

#include "bytestream.h"
#include "iqhandler.h"
#include "messagehandler.h"
#include "gloox.h"

namespace gloox
{

  class BytestreamDataHandler;
  class ClientBase;
  class Message;

  /**
   * @brief An implementation of a single In-Band Bytestream (XEP-0047).
   *
   * One instance of this class handles a single byte stream.
   *
   * See SIProfileFT for a detailed description on how to implement file transfer.
   *
   * @note This class can @b receive data wrapped in Message stanzas. This will only work if you
   * are not using MessageSessions. However, it will always send
   * data using IQ stanzas (which will always work).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API InBandBytestream : public Bytestream, public IqHandler, public MessageHandler
  {

    friend class SIProfileFT;

    public:
      /**
       * Virtual destructor.
       */
      virtual ~InBandBytestream();

      /**
       * Lets you retrieve this bytestream's block-size.
       * @return The bytestream's block-size.
       */
      int blockSize() const { return m_blockSize; }

      /**
       * Sets the stream's block-size. Default: 4096
       * @param blockSize The new block size.
       * @note You should not change the block size once connect() has been called.
       */
      void setBlockSize( int blockSize ) { m_blockSize = blockSize; }

      // reimplemented from Bytestream
      virtual ConnectionError recv( int timeout = -1 ) { (void)timeout; return ConnNoError; }

      // reimplemented from Bytestream
      bool send( const std::string& data );

      // reimplemented from Bytestream
      virtual bool connect();

      // reimplemented from Bytestream
      virtual void close();

      // reimplemented from IqHandler
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler
      virtual void handleIqID( const IQ& iq, int context );

      // reimplemented from MessageHandler
      virtual void handleMessage( const Message& msg, MessageSession* session = 0 );

    private:
#ifdef INBANDBYTESTREAM_TEST
    public:
#endif
      enum IBBType
      {
        IBBOpen,
        IBBData,
        IBBClose,
        IBBInvalid
      };

      /**
       * @brief An abstraction of IBB elements, implemented as as StanzaExtension.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class IBB : public StanzaExtension
      {
        public:
          /**
           * Constructs a new IBB object that opens an IBB, using the given SID and block size.
           * @param sid The SID of the IBB to open.
           * @param blocksize The streams block size.
           */
          IBB( const std::string& sid, int blocksize );

          /**
           * Constructs a new IBB object that can be used to send a single block of data,
           * using the given SID and sequence number.
           * @param sid The SID of the IBB.
           * @param seq The block's sequence number.
           * @param data The block data, not base64 encoded.
           */
          IBB( const std::string& sid, int seq, const std::string& data );

          /**
           * Constructs a new IBB object that closes an IBB, using the given SID.
           * @param sid The SID of the IBB to close.
           */
          IBB( const std::string& sid );

          /**
           * Constructs a new IBB object from the given Tag.
           * @param tag The Tag to parse.
           */
          IBB( const Tag* tag = 0 );

          /**
           * Virtual destructor.
           */
          virtual ~IBB();

          /**
           * Returns the IBB's type.
           * @return The IBB's type.
           */
          IBBType type() const { return m_type; }

          /**
           * Returns the IBB's block size. Only meaningful if the IBB is of type() IBBOpen.
           * @return The IBB's block size.
           */
          int blocksize() const { return m_blockSize; }

          /**
           * Returns the current block's sequence number.
           * @return The current block's sequence number.
           */
          int seq() const { return m_seq; }

          /**
           * Returns the current block's SID.
           * @return The current block's SID.
           */
          const std::string sid() const { return m_sid; }

          /**
           * Returns the current block's data (not base64 encoded).
           * @return The current block's data.
           */
          const std::string& data() const { return m_data; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new IBB( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new IBB( *this );
          }

        private:
          std::string m_sid;
          int m_seq;
          int m_blockSize;
          std::string m_data;
          IBBType m_type;
      };

      InBandBytestream( ClientBase* clientbase, LogSink& logInstance, const JID& initiator,
                        const JID& target, const std::string& sid );
      InBandBytestream& operator=( const InBandBytestream& );
      void closed(); // by remote entity
      void returnResult( const JID& to, const std::string& id );
      void returnError( const JID& to, const std::string& id, StanzaErrorType type, StanzaError error );

      ClientBase* m_clientbase;
      int m_blockSize;
      int m_sequence;
      int m_lastChunkReceived;

  };

}

#endif // INBANDBYTESTREAM_H__
