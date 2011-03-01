/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef REGISTRATIONHANDLER_H__
#define REGISTRATIONHANDLER_H__

#include "oob.h"

#include <string>

namespace gloox
{

  class OOB;
  class JID;
  class DataForm;

  /**
   * Possible results of a XEP-0077 operation.
   */
  enum RegistrationResult
  {
    RegistrationSuccess = 0,           /**< The last operation (account registration, account
                                        * deletion or password change) was successful. */
    RegistrationNotAcceptable,         /**< 406: Not all necessary information provided */
    RegistrationConflict,              /**< 409: Username alreday exists. */
    RegistrationNotAuthorized,         /**< Account removal: Unregistered entity waits too long
                                        * before authentication or performs tasks other than
                                        * authentication after registration.<br>
                                        * Password change: The server or service does not consider
                                        * the channel safe enough to enable a password change. */
    RegistrationBadRequest,            /**< Account removal: The &lt;remove/&gt; element was not
                                        * the only child element of the &lt;query/&gt; element.
                                        * Should not happen when only gloox functions are being
                                        * used.<br>
                                        * Password change: The password change request does not
                                        * contain complete information (both &lt;username/&gt; and
                                        * &lt;password/&gt; are required). */
    RegistrationForbidden,             /**< Account removal: The sender does not have sufficient
                                        * permissions to cancel the registration. */
    RegistrationRequired,              /**< Account removal: The entity sending the remove
                                        * request was not previously registered. */
    RegistrationUnexpectedRequest,     /**< Account removal: The host is an instant messaging
                                        * server and the IQ get does not contain a 'from'
                                        * address because the entity is not registered with
                                        * the server.<br>
                                        * Password change: The host is an instant messaging
                                        * server and the IQ set does not contain a 'from'
                                        * address because the entity is not registered with
                                        * the server. */
    RegistrationNotAllowed,            /**< Password change: The server or service does not allow
                                        * password changes. */
    RegistrationUnknownError           /**< An unknown error condition occured. */
  };

  /**
   * @brief A virtual interface that receives events from an Registration object.
   *
   * Derived classes can be registered as RegistrationHandlers with an
   * Registration object. Incoming results for operations initiated through
   * the Registration object are forwarded to this handler.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.2
   */
  class GLOOX_API RegistrationHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~RegistrationHandler() {}

      /**
       * Reimplement this function to receive results of the
       * @ref Registration::fetchRegistrationFields() function.
       * @param from The server or service the registration fields came from.
       * @param fields The OR'ed fields the server requires. From @ref Registration::fieldEnum.
       * @param instructions Any additional information the server sends along.
       */
      virtual void handleRegistrationFields( const JID& from, int fields,
                                             std::string instructions ) = 0;

      /**
       * This function is called if @ref Registration::createAccount() was called on an authenticated
       * stream and the server lets us know about this.
       */
      virtual void handleAlreadyRegistered( const JID& from ) = 0;

      /**
       * This funtion is called to notify about the result of an operation.
       * @param from The server or service the result came from.
       * @param regResult The result of the last operation.
       */
      virtual void handleRegistrationResult( const JID& from, RegistrationResult regResult ) = 0;

      /**
       * This function is called additionally to @ref handleRegistrationFields() if the server
       * supplied a data form together with legacy registration fields.
       * @param from The server or service the data form came from.
       * @param form The DataForm containing registration information.
       */
      virtual void handleDataForm( const JID& from, const DataForm& form ) = 0;

      /**
       * This function is called if the server does not offer in-band registration
       * but wants to refer the user to an external URL.
       * @param from The server or service the referal came from.
       * @param oob The OOB object describing the external URL.
       */
      virtual void handleOOB( const JID& from, const OOB& oob ) = 0;

  };

}

#endif // REGISTRATIONHANDLER_H__
