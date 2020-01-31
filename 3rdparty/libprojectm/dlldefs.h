/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2007 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */
/**
 * $Id$
 *
 * DLL definitions for exporting symbols on various platforms
 *
 * $Log$
 */

#ifndef _DLLDEFS_H
#define _DLLDEFS_H

#ifndef DLLEXPORT
#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else /** !WIN32 */
#define DLLEXPORT
#endif /** WIN32 */
#endif /** !DLLEXPORT */

#endif /** !_DLLDEFS_H */
