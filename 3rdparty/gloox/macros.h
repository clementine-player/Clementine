/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef GLOOX_MACROS_H__
#define GLOOX_MACROS_H__

#if defined( _MSC_VER ) || defined( _WIN32_WCE )
#  pragma warning( disable:4251 )
#  pragma warning( disable:4786 )
#endif

#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
#  if defined( GLOOX_EXPORTS ) || defined( DLL_EXPORT )
#    define GLOOX_API __declspec( dllexport )
#  else
#    if defined( GLOOX_IMPORTS ) || defined( DLL_IMPORT )
#      define GLOOX_API __declspec( dllimport )
#    endif
#  endif
#endif

#ifndef GLOOX_API
#  define GLOOX_API
#endif


#if defined( __GNUC__ ) && ( __GNUC__ - 0 > 3 || ( __GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2 ) )
#  define GLOOX_DEPRECATED __attribute__ ( (__deprecated__) )
#  define GLOOX_DEPRECATED_CTOR explicit GLOOX_DEPRECATED
#elif defined( _MSC_VER ) && ( _MSC_VER >= 1300 )
#  define GLOOX_DEPRECATED __declspec( deprecated )
#  define GLOOX_DEPRECATED_CTOR explicit GLOOX_DEPRECATED
#else
#  define GLOOX_DEPRECATED
#  define GLOOX_DEPRECATED_CTOR
#endif


#endif // GLOOX_MACROS_H__
