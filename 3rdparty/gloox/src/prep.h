/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef PREP_H__
#define PREP_H__

#include "macros.h"

#include <string>

namespace gloox
{

  /**
   * @brief This namespace offers functions to stringprep the individual parts of a JID.
   *
   * You should not need to use these functions directly. All the
   * necessary prepping is done for you if you stick to the interfaces provided.
   * If you write your own enhancements, check with the spec.
   *
   * @note These functions depend on an installed LibIDN at compile time of gloox. If
   * LibIDN is not installed these functions return the string they are given
   * without any modification.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.2
   */
  namespace prep
  {
    /**
     * This function applies the Nodeprep profile of Stringprep to a string.
     * @param node The string to apply the profile to.
     * @param out The prepped string. In case of an error this string is not touched.
     * If LibIDN is not available the string is returned unchanged.
     * @return @b True if prepping was successful, @b false otherwise or of LibIDN
     * is not available.
     */
    bool nodeprep( const std::string& node, std::string& out );

    /**
     * This function applies the Nameprep profile of Stringprep to a string.
     * @param domain The string to apply the profile to.
     * @param out The prepped string. In case of an error this string is not touched.
     * If LibIDN is not available the string is returned unchanged.
     * @return @b True if prepping was successful, @b false otherwise or of LibIDN
     * is not available.
     */
    bool nameprep( const std::string& domain, std::string& out );

    /**
     * This function applies the Resourceprep profile of Stringprep to a std::string.
     * @param resource The string to apply the profile to.
     * @param out The prepped string. In case of an error this string is not touched.
     * If LibIDN is not available the string is returned unchanged.
     * @return @b True if prepping was successful, @b false otherwise or of LibIDN
     * is not available.
     */
    bool resourceprep( const std::string& resource, std::string& out );

    /**
     * This function applies the idna() function to a string. I.e. it transforms
     * internationalized domain names into plain ASCII.
     * @param domain The string to convert.
     * @param out The converted string. In case of an error this string is not touched.
     * If LibIDN is not available the string is returned unchanged.
     * @return @b True if prepping was successful, @b false otherwise or of LibIDN
     * is not available.
     */
    bool idna( const std::string& domain, std::string& out );

  }

}

#endif // PREP_H__
