%Module clementine 0

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtNetwork/QtNetworkmod.sip

%Include autoexpandingtreeview.sip
%Include directory.sip
%Include engine_fwd.sip
%Include iconloader.sip
%Include librarybackend.sip
%Include libraryquery.sip
%Include libraryview.sip
%Include mergedproxymodel.sip
%Include network.sip
%Include parserbase.sip
%Include player.sip
%Include playlist.sip
%Include playlistitem.sip
%Include playlistmanager.sip
%Include playlistparser.sip
%Include playlistsequence.sip
%Include pythonengine.sip
%Include radiomodel.sip
%Include radioservice.sip
%Include queue.sip
%Include scriptinterface.sip
%Include settingsdialog.sip
%Include song.sip
%Include songinsertvetolistener.sip
%Include songloader.sip
%Include taskmanager.sip
%Include uiinterface.sip

// Remember: when adding a class that inherits from QObject, add it to the list
// in scriptinterface.sip as well, or else it won't cast properly when calling
// python slots.
