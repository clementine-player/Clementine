/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef REGISTRATION_H__
#define REGISTRATION_H__

#include "iqhandler.h"
#include "registrationhandler.h"
#include "dataform.h"
#include "jid.h"
#include "oob.h"

#include <string>
#include <map>

namespace gloox
{

  class ClientBase;
  class Stanza;

  /**
   * Holds all the possible fields a server may require for registration according
   * to Section 14.1, XEP-0077.
   */
  struct RegistrationFields
  {
    std::string username;           /**< Desired username. */
    std::string nick;               /**< User's nickname. */
    std::string password;           /**< User's password. */
    std::string name;               /**< User's name. */
    std::string first;              /**< User's first name.*/
    std::string last;               /**< User's last name. */
    std::string email;              /**< User's email address. */
    std::string address;            /**< User's address. */
    std::string city;               /**< User's city. */
    std::string state;              /**< User's state. */
    std::string zip;                /**< User's ZIP code. */
    std::string phone;              /**< User's phone number. */
    std::string url;                /**< User's homepage URL (or other URL). */
    std::string date;               /**< Date (?) */
    std::string misc;               /**< Misc (?) */
    std::string text;               /**< Text (?)*/
  };

  /**
   * @brief This class is an implementation of XEP-0077 (In-Band Registration).
   *
   * Derive your object from @ref RegistrationHandler and implement the
   * virtual functions offered by that interface. Then use it like this:
   * @code
   * void MyClass::myFunc()
   * {
   *   m_client = new Client( "example.org" );
   *   m_client->disableRoster(); // a roster is not necessary for registration
   *   m_client->registerConnectionListener( this );
   *
   *   m_reg = new Registration( c );
   *   m_reg->registerRegistrationHandler( this );
   *
   *   m_client->connect();
   * }
   *
   * void MyClass::onConnect()
   * {
   *   m_reg->fetchRegistrationFields();
   * }
   * @endcode
   *
   * In RegistrationHandler::handleRegistrationFields() you should check which information the server
   * requires to open a new account. You might not always get away with just username and password.
   * Then call createAccount() with a filled-in RegistrationFields and an @c int representing the bit-wise
   * ORed fields you want to have included in the registration attempt. For your convenience you can
   * use the 'fields' argument of handleRegistrationFields(). ;) It's your responsibility to make
   * sure at least those fields the server requested are filled in.
   *
   * Check @c tests/register_test.cpp for an example.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.2
   */
  class GLOOX_API Registration : public IqHandler
  {
    public:

      /**
       * The possible fields of a XEP-0077 account registration.
       */
      enum fieldEnum
      {
        FieldUsername  =     1,
        FieldNick      =     2,
        FieldPassword  =     4,
        FieldName      =     8,
        FieldFirst     =    16,
        FieldLast      =    32,
        FieldEmail     =    64,
        FieldAddress   =   128,
        FieldCity      =   256,
        FieldState     =   512,
        FieldZip       =  1024,
        FieldPhone     =  2048,
        FieldUrl       =  4096,
        FieldDate      =  8192,
        FieldMisc      = 16384,
        FieldText      = 32768
      };

      /**
       * @brief A wrapping class for the XEP-0077 &lt;query&gt; element.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class Query : public StanzaExtension
      {
        public:
          /**
           * Creates a new object that can be used to carry out a registration.
           * @param form A DataForm containing the registration terms.
           */
          Query( DataForm* form );

          /**
           * Creates a new object that can be used to carry out a registration.
           * @param del Whether or not to remove the account.
           */
          Query( bool del = false );

          /**
           * Creates a new object that can be used to carry out a registration.
           * @param fields Bit-wise ORed fieldEnum values describing the valid (i.e., set)
           * fields in the @b values parameter.
           * @param values Contains the registration fields.
           */
          Query( int fields, const RegistrationFields& values );

          /**
           * Creates a new object from the given Tag.
           * @param tag The Tag to parse.
           */
          Query( const Tag* tag );

          /**
           * Virtual Destructor.
           */
          virtual ~Query();

          /**
           * Returns the contained registration form, if any.
           * @return The registration form. May be 0.
           */
          const DataForm* form() const { return m_form; }

          /**
           * Returns the registration instructions, if given
           * @return The registration instructions.
           */
          const std::string& instructions() const { return m_instructions; }

          /**
           * Returns the registration fields, if set.
           * @return The registration fields.
           */
          int fields() const { return m_fields; }

          /**
           *
           */
          const RegistrationFields& values() const { return m_values; }

          /**
           * Indicates whether the account is already registered.
           * @return @b True if the &lt;registered&gt; element is present, @b false otherwise.
           */
          bool registered() const { return m_reg; }

          /**
           * Indicates whether the account shall be removed.
           * @return @b True if the &lt;remove&gt; element is present, @b false otherwise.
           */
          bool remove() const { return m_del; }

          /**
           * Returns an optional OOB object.
           * @return A pointer to an OOB object, if present, 0 otherwise.
           */
          const OOB* oob() const { return m_oob; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Query( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            Query* q = new Query();
            q->m_form = m_form ? new DataForm( *m_form ) : 0;
            q->m_fields = m_fields;
            q->m_values = m_values;
            q->m_instructions = m_instructions;
            q->m_oob = new OOB( *m_oob );
            q->m_del = m_del;
            q->m_reg = m_reg;
            return q;
          }

        private:
          DataForm* m_form;
          int m_fields;
          RegistrationFields m_values;
          std::string m_instructions;
          OOB* m_oob;
          bool m_del;
          bool m_reg;
      };

      /**
       * Constructor.
       * @param parent The ClientBase which is used for establishing a connection.
       * @param to The server or service to authenticate with. If empty the currently connected
       * server will be used.
       */
      Registration( ClientBase* parent, const JID& to );

      /**
       * Constructor. Registration will be attempted with the ClientBase's connected host.
       * @param parent The ClientBase which is used for establishing a connection.
       */
      Registration( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~Registration();

      /**
       * Use this function to request the registration fields the server requires.
       * The required fields are returned asynchronously to the object registered as
       * @ref RegistrationHandler by calling @ref RegistrationHandler::handleRegistrationFields().
       */
      void fetchRegistrationFields();

      /**
       * Attempts to register an account with the given credentials. Only the fields OR'ed in
       * @c fields will be sent. This can only be called with an unauthenticated parent (@ref Client).
       * @note It is recommended to use @ref fetchRegistrationFields to find out which fields the
       * server requires.
       * @param fields The fields to use to generate the registration request. OR'ed
       * @ref fieldEnum values.
       * @param values The struct contains the values which shall be used for the registration.
       * @return Returns @b true if the registration request was sent successfully, @b false
       * otherwise. In that case either there's no connected ClientBase available, or
       * prepping of the username failed (i.e. the username is not valid for use in XMPP).
       */
      bool createAccount( int fields, const RegistrationFields& values );

      /**
       * Attempts to register an account with the given credentials. This can only be called with an
       * unauthenticated parent (@ref Client).
       * @note According to XEP-0077, if the server sends both old-style fields and data form,
       * implementations SHOULD prefer data forms.
       * @param form The DataForm containing the registration credentials.
       */
      void createAccount( DataForm* form );

      /**
       * Tells the server to remove the currently authenticated account from the server.
       */
      void removeAccount();

      /**
       * Tells the server to change the password for the current account.
       * @param username The username to change the password for. You might want to use
       * Client::username() to get the current prepped username.
       * @param password The new password.
       */
      void changePassword( const std::string& username, const std::string& password );

      /**
       * Registers the given @c rh as RegistrationHandler. Only one handler is possible at a time.
       * @param rh The RegistrationHandler to register.
       */
      void registerRegistrationHandler( RegistrationHandler* rh );

      /**
       * Un-registers the current RegistrationHandler.
       */
      void removeRegistrationHandler();

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
#ifdef REGISTRATION_TEST
    public:
#endif

      enum IdType
      {
        FetchRegistrationFields,
        CreateAccount,
        RemoveAccount,
        ChangePassword
      };

      Registration operator=( const Registration& );

      void init();

      ClientBase* m_parent;
      const JID m_to;
      RegistrationHandler* m_registrationHandler;
  };

}

#endif // REGISTRATION_H__
