/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STATISTICSHANDLER_H__
#define STATISTICSHANDLER_H__

#include "stanza.h"

namespace gloox
{

  /**
   * A structure describing the current connection statistics.
   */
  struct StatisticsStruct
  {
    long int totalBytesSent;             /**< The total number of bytes sent over the wire. This does @b not
                                     * include the TLS handshake nor any TLS-related overhead, but it does
                                     * include anything sent before compression was switched on. */
    long int totalBytesReceived;         /**< The total number of bytes received over the wire. This does @b not
                                     * include the TLS handshake nor any TLS-related overhead, but it does
                                     * include anything sent before compression was switched on. */
    long int compressedBytesSent;        /**< Total number of bytes sent over the wire after compression was
                                     * applied. */
    long int compressedBytesReceived;    /**< Total number of bytes received over the wire before decompression
                                     * was applied. */
    long int uncompressedBytesSent;      /**< Total number of bytes sent over the wire before compression was
                                     * applied. */
    long int uncompressedBytesReceived;  /**< Total number of bytes received over the wire after decompression
                                     * was applied. */
    long int totalStanzasSent;           /**< The total number of Stanzas sent. */
    long int totalStanzasReceived;       /**< The total number of Stanzas received. */
    long int iqStanzasSent;              /**< The total number of IQ Stanzas sent. */
    long int iqStanzasReceived;          /**< The total number of IQ Stanzas received. */
    long int messageStanzasSent;         /**< The total number of Message Stanzas sent. */
    long int messageStanzasReceived;     /**< The total number of Message Stanzas received. */
    long int s10nStanzasSent;            /**< The total number of Subscription Stanzas sent. */
    long int s10nStanzasReceived;        /**< The total number of Subscription Stanzas received. */
    long int presenceStanzasSent;        /**< The total number of Presence Stanzas sent. */
    long int presenceStanzasReceived;    /**< The total number of Presence Stanzas received. */
    bool encryption;                /**< Whether or not the connection (to the server) is encrypted. */
    bool compression;               /**< Whether or not the stream (to the server) gets compressed. */
  };

  /**
   * @brief A virtual interface which can be reimplemented to receive connection statistics.
   *
   * Derived classes can be registered as StatisticsHandlers with the ClientBase.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API StatisticsHandler
  {
     public:
      /**
       * Virtual Destructor.
       */
       virtual ~StatisticsHandler() {}

       /**
        * This function is called when a Stanza has been sent or received.
        * @param stats The updated connection statistics.
        */
       virtual void handleStatistics( const StatisticsStruct stats ) = 0;
  };

}

#endif // STATISTICSHANDLER_H__
