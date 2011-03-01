/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ADHOC_H__
#define ADHOC_H__

#include "dataform.h"
#include "disco.h"
#include "disconodehandler.h"
#include "discohandler.h"
#include "iqhandler.h"
#include "stanzaextension.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class ClientBase;
  class Stanza;
  class AdhocHandler;
  class AdhocCommandProvider;

  /**
   * @brief This class implements a provider for XEP-0050 (Ad-hoc Commands).
   *
   * The current, not complete, implementation is probably best suited for fire-and-forget
   * type of commands. Any additional feature, like multiple stages, etc., would have to be
   * added separately.
   *
   * To offer commands to remote entities, use this class as follows:<br>
   * Create a class that will handle command execution requests and derive it from
   * AdhocCommandProvider. Instantiate an Adhoc object and register your
   * AdhocCommandProvider-derived object with the Adhoc object using
   * registerAdhocCommandProvider(). The additional parameters to that method are the internal
   * name of the command as used in the code, and the public name of the command as it
   * will be shown to an end user:
   * @code
   * MyClass::someFunc()
   * {
   *   Adhoc* m_adhoc = new Adhoc( m_client );
   *
   *   // this might be a bot monitoring a weather station, for example
   *   m_adhoc->registerAdhocCommandProvider( this, "getTemp", "Retrieve current temperature" );
   *   m_adhoc->registerAdhocCommandProvider( this, "getPressure", "Retrieve current air pressure" );
   *   [...]
   * }
   * @endcode
   * In this example, MyClass is AdhocCommandProvider-derived so it is obviously the command handler, too.
   *
   * And that's about it you can do with the Adhoc class. Of course you can have a AdhocCommandProvider
   * handle more than one command, just register it with the Adhoc object for every desired command,
   * like shown above.
   *
   * What the Adhoc object does when you install a new command is tell the supplied Disco object
   * to advertise these commands to clients using the 'Service Discovery' protocol to learn about
   * this implementation's features. These clients can then call and execute the command. Of course you
   * are free to implement access restrictions to not let anyone mess with your bot, for example.
   * However, the commands offered using Service Discovery are publically visible in any case.
   *
   * To execute commands offered by a remote entity:<br>
   * ...TBC...
   *
   * XEP version: 1.2
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API Adhoc : public DiscoNodeHandler, public DiscoHandler, public IqHandler
  {
    public:
      /**
       * @brief An abstraction of an Adhoc Command element (from Adhoc Commands, XEP-0050)
       * as a StanzaExtension.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class GLOOX_API Command : public StanzaExtension
      {
        friend class Adhoc;

        public:

          /**
           * Specifies the action to undertake with the given command.
           */
          enum Action
          {
            Execute       =  1,     /**< The command should be executed or continue to be executed.
                                     * This is the default value. */
            Cancel        =  2,     /**< The command should be canceled. */
            Previous      =  4,     /**< The command should be digress to the previous stage of
                                     * execution. */
            Next          =  8,     /**< The command should progress to the next stage of
                                     * execution. */
            Complete      = 16,     /**< The command should be completed (if possible). */
            InvalidAction = 32      /**< The action is unknown or invalid. */
          };

          /**
           * Describes the current status of a command.
           */
          enum Status
          {
            Executing,              /**< The command is being executed. */
            Completed,              /**< The command has completed. The command session has ended. */
            Canceled,               /**< The command has been canceled. The command session has ended. */
            InvalidStatus           /**< The status is unknown or invalid. */
          };

          /**
           * An abstraction of a command note.
           *
           * @author Jakob Schroeter <js@camaya.net>
           * @since 1.0
           */
          class GLOOX_API Note
          {

            friend class Command;

            public:
              /**
               * Specifies the severity of a note.
               */
              enum Severity
              {
                Info,               /**< The note is informational only. This is not really an
                                     * exceptional condition. */
                Warning,            /**< The note indicates a warning. Possibly due to illogical
                                     * (yet valid) data. */
                Error,              /**< The note indicates an error. The text should indicate the
                                     * reason for the error. */
                InvalidSeverity     /**< The note type is unknown or invalid. */
              };

              /**
               * A convenience constructor.
               * @param sev The note's severity.
               * @param note The note's content.
               */
              Note( Severity sev, const std::string& note )
                : m_severity( sev ), m_note( note ) {}

              /**
               * Destructor.
               */
              ~Note() {}

              /**
               * Returns the note's severity.
               * @return The note's severity.
               */
              Severity severity() const { return m_severity; }

              /**
               * Returns the note's content.
               * @return The note's content.
               */
              const std::string& content() const { return m_note; }

              /**
               * Returns a Tag representation of the Note.
               * @return A Tag representation.
               */
              Tag* tag() const;

            private:
#ifdef ADHOC_COMMANDS_TEST
            public:
#endif
              /**
               * Constructs a new Note from the given Tag.
               * @param tag The Tag to parse.
               */
              Note( const Tag* tag );

              Severity m_severity;      /**< The note's severity. */
              std::string m_note;       /**< The note's content. */
          };

          /**
           * A list of command notes.
           */
          typedef std::list<const Note*> NoteList;

          /**
           * Creates a Command object that can be used to perform the provided Action.
           * This constructor is used best to continue execution of a multi stage command
           * (for which the session ID must be known).
           * @param node The node (command) to perform the action on.
           * @param sessionid The session ID of an already running adhoc command session.
           * @param action The action to perform.
           * @param form An optional DataForm to include in the request. Will be deleted in Command's
           * destructor.
           */
          Command( const std::string& node, const std::string& sessionid, Action action,
                   DataForm* form = 0 );

          /**
           * Creates a Command object that can be used to perform the provided Action.
           * This constructor is used best to reply to an execute request.
           * @param node The node (command) to perform the action on.
           * @param sessionid The (possibly newly created) session ID of the adhoc command session.
           * @param status The execution status.
           * @param form An optional DataForm to include in the reply. Will be deleted in Command's
           * destructor.
           */
          Command( const std::string& node, const std::string& sessionid, Status status,
                   DataForm* form = 0 );

          /**
           * Creates a Command object that can be used to perform the provided Action.
           * This constructor is used best to reply to a multi stage command that is not yet completed
           * (for which the session ID must be known).
           * @param node The node (command) to perform the action on.
           * @param sessionid The (possibly newly created) session ID of the adhoc command session.
           * @param status The execution status.
           * @param executeAction The action to execute.
           * @param allowedActions Allowed reply actions.
           * @param form An optional DataForm to include in the reply. Will be deleted in Command's
           * destructor.
           */
          Command( const std::string& node, const std::string& sessionid, Status status,
                   Action executeAction, int allowedActions = Complete,
                   DataForm* form = 0 );

          /**
           * Creates a Command object that can be used to perform the provided Action.
           * This constructor is used best to execute the initial step of a command
           * (single or multi stage).
           * @param node The node (command) to perform the action on.
           * @param action The action to perform.
           * @param form An optional DataForm to include in the request. Will be deleted in Command's
           * destructor.
           */
          Command( const std::string& node, Action action,
                   DataForm* form = 0 );

          /**
           * Creates a Command object from the given Tag.
           * @param tag A &lt;command&gt; tag in the adhoc commands' namespace.
           */
          Command( const Tag* tag = 0 );

          /**
           * Virtual destructor.
           */
          virtual ~Command();

          /**
           * Returns the node identifier (the command).
           * @return The node identifier.
           */
          const std::string& node() const { return m_node; }

          /**
           * Returns the command's session ID, if any.
           * @return The command's session ID.
           */
          const std::string& sessionID() const { return m_sessionid; }

          /**
           * Returns the execution status for a command. Only valid for execution
           * results.
           * @return The execution status for a command.
           */
          Status status() const { return m_status; }

          /**
           * Returns the command's action.
           * @return The command's action.
           */
          Action action() const { return m_action; }

          /**
           * Returns the ORed actions that are allowed to be executed on the
           * current stage.
           * @return An int containing the ORed actions.
           */
          int actions() const { return m_actions; }

          /**
           * Returns the list of notes associated with the command.
           * @return The list of notes.
           */
          const NoteList& notes() const { return m_notes; }

          /**
           * Use this function to add a note to the command.
           * @param note A pointer to a Note object. The Command will own
           * the Note.
           */
          void addNote( const Note* note ) { m_notes.push_back( note ); }

          /**
           * Returns the command's embedded DataForm.
           * @return The command's embedded DataForm. May be 0.
           */
          const DataForm* form() const { return m_form; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Command( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            Command* c = new Command();

            NoteList::const_iterator it = m_notes.begin();
            for( ; it != m_notes.end(); ++it )
              c->m_notes.push_back( new Note( *(*it) ) );

            c->m_node = m_node;
            c->m_sessionid = m_sessionid;
            c->m_form = m_form ? static_cast<DataForm*>( m_form->clone() ) : 0;
            c->m_action = m_action;
            c->m_status = m_status;
            c->m_actions = m_actions;

            return c;
          }

        private:
#ifdef ADHOC_COMMANDS_TEST
        public:
#endif
          NoteList m_notes;

          std::string m_node;
          std::string m_sessionid;
          DataForm* m_form;
          Action m_action;
          Status m_status;
          int m_actions;
      };

      /**
       * Constructor.
       * Creates a new Adhoc client that registers as IqHandler with a ClientBase.
       * @param parent The ClientBase used for XMPP communication.
       */
      Adhoc( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~Adhoc();

      /**
       * This function queries the given remote entity for Adhoc Commands support.
       * @param remote The remote entity's JID.
       * @param ah The object handling the result of this request.
       */
      void checkSupport( const JID& remote, AdhocHandler* ah );

      /**
       * Retrieves a list of commands from the remote entity. You should check whether the remote
       * entity actually supports Adhoc Commands by means of checkSupport().
       * @param remote The remote entity's JID.
       * @param ah The object handling the result of this request.
       */
      void getCommands( const JID& remote, AdhocHandler* ah );

      /**
       * Executes or continues the given command on the given remote entity.
       * To construct the @c command object, it is recommended to use either
       * Command( const std::string&, Action ) to begin execution of a command, or
       * Command( const std::string&, const std::string&, Action ) to continue execution
       * of a command.
       * @param remote The remote entity's JID.
       * @param command The command to execute.
       * @param ah The object handling the result of this request.
       */
      void execute( const JID& remote, const Adhoc::Command* command, AdhocHandler* ah );

      /**
       * Use this function to respond to an execution request submitted by means
       * of AdhocCommandProvider::handleAdhocCommand().
       * It is recommended to use
       * Command( const std::string&, const std::string&, Status, DataForm* )
       * to construct the @c command object.
       * Optionally, an Error object can be included. In that case the IQ sent is of type @c error.
       * @param remote The requester's JID.
       * @param command The response. The Adhoc object will own and delete the
       * command object pointed to.
       * @param error An optional Error obejct to include.
       */
      void respond( const JID& remote, const Adhoc::Command* command, const Error* error = 0 );

      /**
       * Using this function, you can register a AdhocCommandProvider -derived object as
       * handler for a specific Ad-hoc Command as defined in XEP-0050.
       * @param acp The obejct to register as handler for the specified command.
       * @param command The node name of the command. Will be announced in disco#items.
       * @param name The natural-language name of the command. Will be announced in disco#items.
       */
      void registerAdhocCommandProvider( AdhocCommandProvider* acp, const std::string& command,
                                         const std::string& name );

      /**
       * Use this function to unregister an adhoc command previously registered using
       * registerAdhocCommandProvider().
       * @param command The command to unregister.
       */
      void removeAdhocCommandProvider( const std::string& command );

      // reimplemented from DiscoNodeHandler
      virtual StringList handleDiscoNodeFeatures( const JID& from, const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual Disco::IdentityList handleDiscoNodeIdentities( const JID& from,
          const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual Disco::ItemList handleDiscoNodeItems( const JID& from, const JID& to, const std::string& node );

      // reimplemented from IqHandler
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler
      virtual void handleIqID( const IQ& iq, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoInfo( const JID& from, const Disco::Info& info, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoItems( const JID& from, const Disco::Items& items, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoError( const JID& from, const Error* error, int context );

    private:
#ifdef ADHOC_TEST
    public:
#endif
      typedef std::map<const std::string, AdhocCommandProvider*> AdhocCommandProviderMap;
      AdhocCommandProviderMap m_adhocCommandProviders;

      enum AdhocContext
      {
        CheckAdhocSupport,
        FetchAdhocCommands,
        ExecuteAdhocCommand
      };

      struct TrackStruct
      {
        JID remote;
        AdhocContext context;
        std::string session;
        AdhocHandler* ah;
      };
      typedef std::map<std::string, TrackStruct> AdhocTrackMap;
      AdhocTrackMap m_adhocTrackMap;

      ClientBase* m_parent;

      StringMap m_items;
      StringMap m_activeSessions;

  };

}

#endif // ADHOC_H__
