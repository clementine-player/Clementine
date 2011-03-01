#include "../client.h"
#include "../connectionlistener.h"
#include "../annotationshandler.h"
#include "../disco.h"
#include "../annotations.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

class AnnotationsTest : public AnnotationsHandler, ConnectionListener
{
  public:
    AnnotationsTest() {}
    virtual ~AnnotationsTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );

      j->registerConnectionListener(this );
      j->disco()->setVersion( "annotationsTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      a = new Annotations( j );
      a->registerAnnotationsHandler( this );

      j->connect();

      delete( a );
      delete( j );
    }

    virtual void onConnect()
    {
      a->requestAnnotations();
    }

    virtual void onDisconnect( ConnectionError /*e*/ ) { printf( "annotations_test: disconnected\n" ); }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void handleAnnotations( const AnnotationsList &aList )
    {
      printf( "received notes...\n" );
      AnnotationsList::const_iterator it = aList.begin();
      for( ; it != aList.end(); it++ )
      {
        printf( "jid: %s, note: %s, cdate: %s, mdate: %s\n", (*it).jid.c_str(),
                (*it).note.c_str(), (*it).cdate.c_str(), (*it).mdate.c_str() );
      }

      AnnotationsList mybList;

      AnnotationsListItem bItem;
      bItem.jid = "romeo@montague.org";
      bItem.note = "my lover & friend. 2 > 3";
      mybList.push_back( bItem );

      bItem.jid = "juliet@capulet.com";
      bItem.note = "oh my sweetest love...";
      bItem.cdate = "20040924T15:23:21";
      bItem.mdate = "20040924T15:23:21";
      mybList.push_back( bItem );

      a->storeAnnotations( mybList );
    }

  private:
    Client *j;
    Annotations *a;
};

int main( int /*argc*/, char** /*argv*/ )
{
  AnnotationsTest *t = new AnnotationsTest();
  t->start();
  delete( t );
  return 0;
}
