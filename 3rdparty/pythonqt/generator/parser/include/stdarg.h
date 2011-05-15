/****************************************************************************
**
** Copyright (C) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Script Generator project on Qt Labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef __STDARG
#define __STDARG

#if !defined(_VA_LIST) && !defined(__VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED

typedef char *__va_list;
#endif
static float __va_arg_tmp;
typedef __va_list va_list;

#define va_start(list, start)               ((void)0)
#define __va_arg(list, mode, n)             ((void)0)
#define _bigendian_va_arg(list, mode, n)    ((void)0)
#define _littleendian_va_arg(list, mode, n) ((void)0)
#define va_end(list)                        ((void)0)
#define va_arg(list, mode)                  ((void)0)

typedef void *__gnuc_va_list;

#endif
