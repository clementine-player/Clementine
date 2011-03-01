#include "../client.h"
#include "../connectionlistener.h"
#include "../bookmarkhandler.h"
#include "../disco.h"
#include "../bookmarkstorage.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class BookmarkStorageTest : public BookmarkHandler, ConnectionListener
{
  public:
    BookmarkStorageTest() {}
    virtual ~BookmarkStorageTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );

      j->registerConnectionListener(this );
      j->disco()->setVersion( "bookmarkTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      b = new BookmarkStorage( j );
      b->registerBookmarkHandler( this );

      j->connect();

      delete( b );
      delete( j );
    }

    virtual void onConnect()
    {
      b->requestBookmarks();
    }

    virtual void onDisconnect( ConnectionError /*e*/ ) { printf( "disco_test: disconnected\n" ); }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void handleBookmarks( const BookmarkList &bList, const ConferenceList &cList )
    {
      printf( "received bookmarks...\n" );

      BookmarkList::const_iterator it_b = bList.begin();
      for( ; it_b != bList.end(); it_b++ )
      {
        printf( "url: %s, name: %s\n", (*it_b).url.c_str(), (*it_b).name.c_str() );
      }
      ConferenceList::const_iterator it_c = cList.begin();
      for( ; it_c != cList.end(); it_c++ )
      {
        printf( "jid: %s, name: %s, nick: %s, pwd: %s\n", (*it_c).jid.c_str(), (*it_c).name.c_str(),
                (*it_c).nick.c_str(), (*it_c).password.c_str() );
      }

      BookmarkList mybList;
      ConferenceList mycList;

      BookmarkListItem bItem;
      bItem.url = "http://camaya.net/gloox";
      bItem.name = "gloox";
      mybList.push_back( bItem );

      bItem.url = "http://jabber.cc";
      bItem.name = "public jabber services";
      mybList.push_back( bItem );

      ConferenceListItem cItem;
      cItem.jid = "jdev@conference.jabber.org";
      cItem.name = "jabber development";
      cItem.nick = "myNick";
      cItem.autojoin = false;
      mycList.push_back( cItem );

      cItem.jid = "jabberd@conference.jabber.org";
      cItem.name = "jabberd development";
      cItem.nick = "myOtherNick";
      cItem.password = "my password";
      cItem.autojoin = true;
      mycList.push_back( cItem );

      b->storeBookmarks( mybList, mycList );
    }

  private:
    Client *j;
    BookmarkStorage *b;
};

int main( int /*argc*/, char** /*argv*/ )
{
  BookmarkStorageTest *t = new BookmarkStorageTest();
  t->start();
  delete( t );
  return 0;
}
