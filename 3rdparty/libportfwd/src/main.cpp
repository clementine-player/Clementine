#include "portfwd/portfwd.h"
int main(int argc, char** argv)
{
    if(argc!=2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    Portfwd pf;
    if(!pf.init(2000))
    {
        printf("Portfwd.init() failed.\n");
        return 2;
    }
    printf("External IP: %s\n", pf.external_ip().c_str());
    printf("LAN IP: %s\n", pf.lan_ip().c_str());
    printf("Max upstream: %d bps, max downstream: %d bps\n",
           pf.max_upstream_bps(), pf.max_downstream_bps() );
           
    printf("%s\n", ((pf.add( port ))?"Added":"Failed to add") );

    printf("Any key to exit...\n");
    char foo;
    scanf("%c",&foo);

    printf("%s\n",  ((pf.remove( port ))?"Removed.":"Failed to remove") );
    return 0;
}

