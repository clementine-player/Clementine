##
## Process this file with automake to produce Makefile.in
##

SUBDIRS = . $(MAYBE_TESTS) $(MAYBE_EXAMPLES)
DIST_SUBDIRS = . tests examples

AM_CPPFLAGS = -Wall -pedantic -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith \
                -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual \
                -Wwrite-strings -Wformat-security -Wmissing-format-attribute -Wno-long-long

lib_LTLIBRARIES = libgloox.la

libgloox_la_SOURCES = jid.cpp parser.cpp connectiontcpclient.cpp clientbase.cpp tag.cpp stanza.cpp logsink.cpp \
                        dns.cpp prep.cpp base64.cpp client.cpp component.cpp \
                        disco.cpp adhoc.cpp privatexml.cpp registration.cpp \
                        nonsaslauth.cpp rosteritem.cpp rostermanager.cpp privacyitem.cpp \
                        bookmarkstorage.cpp annotations.cpp privacymanager.cpp lastactivity.cpp \
                        flexoff.cpp dataform.cpp dataformfield.cpp dataformfieldcontainer.cpp \
                        messagesession.cpp messageeventfilter.cpp chatstatefilter.cpp gloox.cpp \
                        inbandbytestream.cpp messagefilter.cpp vcard.cpp \
                        vcardmanager.cpp md5.cpp sha.cpp search.cpp dataformreported.cpp dataformitem.cpp \
                        mucroom.cpp mucmessagesession.cpp oob.cpp vcardupdate.cpp stanzaextensionfactory.cpp \
                        delayeddelivery.cpp gpgencrypted.cpp gpgsigned.cpp \
                        uniquemucroom.cpp instantmucroom.cpp compressionzlib.cpp tlsgnutlsclient.cpp \
                        connectionhttpproxy.cpp tlsgnutlsserveranon.cpp tlsgnutlsbase.cpp \
                        tlsgnutlsclientanon.cpp tlsschannel.cpp tlsdefault.cpp simanager.cpp siprofileft.cpp \
                        mutex.cpp connectionsocks5proxy.cpp socks5bytestreammanager.cpp socks5bytestream.cpp \
                        connectiontcpbase.cpp connectiontcpserver.cpp socks5bytestreamserver.cpp amp.cpp \
                        pubsubitem.cpp pubsubmanager.cpp \
                        error.cpp util.cpp iq.cpp message.cpp presence.cpp \
                        subscription.cpp capabilities.cpp chatstate.cpp connectionbosh.cpp connectiontls.cpp \
                        messageevent.cpp receipt.cpp nickname.cpp eventdispatcher.cpp \
                        pubsubevent.cpp xhtmlim.cpp featureneg.cpp \
                        shim.cpp softwareversion.cpp attention.cpp \
                        tlsopensslclient.cpp tlsopensslbase.cpp \
                        tlsopensslserver.cpp compressiondefault.cpp \
                        connectiontlsserver.cpp

libgloox_la_LDFLAGS = -version-info 8:0:0 -no-undefined -no-allow-shlib-undefined
libgloox_la_LIBADD =
libgloox_la_CFLAGS = $(CPPFLAGS)

libglooxincludedir = $(includedir)/gloox
libglooxinclude_HEADERS = adhoccommandprovider.h      privacymanager.h        tag.h \
                            adhoc.h attention.h iqhandler.h             privatexml.h \
                            annotations.h             client.h                privatexmlhandler.h \
                            annotationshandler.h      component.h             registration.h \
                            bookmarkhandler.h         registrationhandler.h   jid.h \
                            bookmarkstorage.h         messagehandler.h        rosteritem.h \
                            clientbase.h              rosterlistener.h        taghandler.h \
                            connectionlistener.h      rostermanager.h         gloox.h \
                            glooxversion.h \
                            disco.h                   presencehandler.h       subscriptionhandler.h \
                            discohandler.h            privacyitem.h           stanza.h \
                            disconodehandler.h        privacylisthandler.h    loghandler.h \
                            lastactivity.h            lastactivityhandler.h   flexoff.h \
                            flexoffhandler.h          dataform.h              dataformfield.h \
                            dataformitem.h            dataformfieldcontainer.h dataformreported.h \
                            macros.h                  logsink.h               messagesession.h \
                            messageeventhandler.h     messagesessionhandler.h chatstatehandler.h \
                            chatstatefilter.h         messageeventfilter.h    inbandbytestream.h \
                            messagefilter.h           vcard.h \
                            vcardmanager.h            vcardhandler.h          adhochandler.h \
                            search.h                  searchhandler.h         statisticshandler.h \
                            resource.h                mucroom.h               mucroomhandler.h \
                            mucroomconfighandler.h    parser.h                \
                            mucinvitationhandler.h    stanzaextension.h       oob.h \
                            vcardupdate.h             delayeddelivery.h       base64.h \
                            gpgencrypted.h            gpgsigned.h \
                            md5.h                     sha.h                   uniquemucroom.h \
                            instantmucroom.h          tlsbase.h               tlshandler.h \
                            connectionbase.h          connectiondatahandler.h compressiondatahandler.h \
                            compressionbase.h         connectiontcpclient.h   connectionhttpproxy.h \
                            tlsdefault.h              simanager.h             siprofilehandler.h \
                            sihandler.h               siprofileft.h           siprofilefthandler.h \
                            socks5bytestreammanager.h connectionsocks5proxy.h event.h \
                            socks5bytestream.h        socks5bytestreamserver.h \
                            softwareversion.h \
                            eventhandler.h \
                            mutex.h                   mutexguard.h            connectiontcpbase.h \
                            connectiontcpserver.h     amp.h                   pubsubmanager.h \
                            iq.h                      pubsub.h                \
                            pubsubresulthandler.h     error.h                 chatstate.h \
                            connectionhandler.h \
                            message.h                 presence.h              subscription.h \
                            bytestream.h              bytestreamdatahandler.h bytestreamhandler.h \
                            capabilities.h            connectionbosh.h        featureneg.h \
                            connectiontls.h           messageevent.h          receipt.h \
                            nickname.h                pubsubevent.h           xhtmlim.h \
                            eventdispatcher.h         \
                            pubsubitem.h shim.h util.h \
                            connectiontlsserver.h compressiondefault.h

noinst_HEADERS = config.h prep.h dns.h nonsaslauth.h mucmessagesession.h stanzaextensionfactory.h \
                   tlsgnutlsclient.h \
                   tlsgnutlsbase.h tlsgnutlsclientanon.h tlsgnutlsserveranon.h tlsopensslbase.h tlsschannel.h \
                   compressionzlib.h rosteritemdata.h tlsopensslclient.h \
                   tlsopensslserver.h

EXTRA_DIST = version.rc

test:
	make -C tests test

check: test

leaktest:
	make -C tests leaktest

leakcheck: leaktest

includetest:
	@for i in *.h; do \
	  echo -e "#include \"$$i\"\nint main() {}" >tmp.cpp; \
	  $(CXX) -I.. tmp.cpp 2>&1 | sed "s/^$$i://g" >$$i.test.log; \
	  test -s $$i.test.log && echo "INC $$i: failed" || rm $$i.test.log; \
	done
	@rm tmp.cpp
	@ls *.h.test.log &>/dev/null || echo "INC all: ok"
