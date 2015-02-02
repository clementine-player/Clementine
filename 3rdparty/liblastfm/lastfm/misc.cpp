/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "misc.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDebug>
#ifdef WIN32
    #include <shlobj.h>
#endif


#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>

QDir
lastfm::dir::bundle()
{
    // Trolltech provided example
    CFURLRef appUrlRef = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
    CFStringRef macPath = CFURLCopyFileSystemPath( appUrlRef, kCFURLPOSIXPathStyle );
    QString path = CFStringToQString( macPath );
    CFRelease(appUrlRef);
    CFRelease(macPath);
    return QDir( path );
}
#endif


static QDir dataDotDot()
{
#ifdef WIN32
    if ((QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) == 0)
    {
        // Use this for non-DOS-based Windowses
        char path[MAX_PATH];
        HRESULT h = SHGetFolderPathA( NULL, 
                                      CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                                      NULL, 
                                      0, 
                                      path );
        if (h == S_OK)
            return QString::fromLocal8Bit( path );
    }
    return QDir::home();
#elif defined(Q_OS_MAC)
    return QDir::home().filePath( "Library/Application Support" );
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    return QDir::home().filePath( ".local/share" );
#else
    return QDir::home();
#endif
}


QDir
lastfm::dir::runtimeData()
{
    return dataDotDot().filePath( "Last.fm" );
}


QDir
lastfm::dir::logs()
{
#ifdef Q_OS_MAC
    return QDir::home().filePath( "Library/Logs/Last.fm" );
#else
    return runtimeData();    
#endif
}


QDir
lastfm::dir::cache()
{
#ifdef Q_OS_MAC
    return QDir::home().filePath( "Library/Caches/Last.fm" );
#else
    return runtimeData().filePath( "cache" );
#endif
}


#ifdef WIN32
QDir
lastfm::dir::programFiles()
{
    char path[MAX_PATH];

    // TODO: this call is dependant on a specific version of shell32.dll.
    // Need to degrade gracefully. Need to bundle SHFolder.exe with installer
    // and execute it on install for this to work on Win98.
    HRESULT h = SHGetFolderPathA( NULL,
                                 CSIDL_PROGRAM_FILES, 
                                 NULL,
                                 0, // current path
                                 path );

    if (h != S_OK)
    {
        qCritical() << "Couldn't get Program Files dir. Possibly Win9x?";
        return QDir();
    }

    return QString::fromLocal8Bit( path );
}
#endif

#ifdef Q_OS_MAC
CFStringRef
lastfm::QStringToCFString( const QString &s )
{
    return CFStringCreateWithCharacters( 0, (UniChar*)s.unicode(), s.length() );
}

QByteArray
lastfm::CFStringToUtf8( CFStringRef s )
{
    QByteArray result;

    if (s != NULL) 
    {
        CFIndex length;
        length = CFStringGetLength( s );
        length = CFStringGetMaximumSizeForEncoding( length, kCFStringEncodingUTF8 ) + 1;
        char* buffer = new char[length];

        if (CFStringGetCString( s, buffer, length, kCFStringEncodingUTF8 ))
            result = QByteArray( buffer );
        else
            qWarning() << "CFString conversion failed.";

        delete[] buffer;
    }

    return result;
}
#endif


const char*
lastfm::platform()
{
#ifdef Q_OS_WIN
    switch (QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_32s:        return "Windows 3.1 with Win32s";
        case QSysInfo::WV_95:         return "Windows 95";
        case QSysInfo::WV_98:         return "Windows 98";
        case QSysInfo::WV_Me:         return "Windows Me";
        case QSysInfo::WV_DOS_based:  return "MS-DOS-based Windows";

        case QSysInfo::WV_NT:         return "Windows NT";
        case QSysInfo::WV_2000:       return "Windows 2000";
        case QSysInfo::WV_XP:         return "Windows XP";
        case QSysInfo::WV_2003:       return "Windows Server 2003";
        case QSysInfo::WV_VISTA:      return "Windows Vista";
        case QSysInfo::WV_WINDOWS7:   return "Windows 7";
        case QSysInfo::WV_NT_based:   return "NT-based Windows";

        case QSysInfo::WV_CE:         return "Windows CE";
        case QSysInfo::WV_CENET:      return "Windows CE.NET";
        case QSysInfo::WV_CE_based:   return "CE-based Windows";

        default:                      return "Unknown";
    }
#elif defined Q_OS_MAC
    switch (QSysInfo::MacintoshVersion)
    {
        case QSysInfo::MV_Unknown:    return "Unknown Mac";
        case QSysInfo::MV_9:          return "Mac OS 9";
        case QSysInfo::MV_10_0:       return "Mac OS X 10.0";
        case QSysInfo::MV_10_1:       return "Mac OS X 10.1";
        case QSysInfo::MV_10_2:       return "Mac OS X 10.2";
        case QSysInfo::MV_10_3:       return "Mac OS X 10.3";
        case QSysInfo::MV_10_4:       return "Mac OS X 10.4";
        case QSysInfo::MV_10_5:       return "Mac OS X 10.5";
        case QSysInfo::MV_10_6:       return "Mac OS X 10.6";
        case QSysInfo::MV_10_7:       return "Mac OS X 10.7";
        case QSysInfo::MV_10_8:       return "Mac OS X 10.8";

        default:                      return "Unknown";
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    return "UNIX X11";
#else
    return "Unknown";
#endif
}

QString lastfm::
md5( const QByteArray& src )
{
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() ).rightJustified( 32, '0' ).toLower();
}

#ifdef Q_OS_MAC
QString
lastfm::CFStringToQString( CFStringRef s )
{
    return QString::fromUtf8( CFStringToUtf8( s ) );
}
#endif

