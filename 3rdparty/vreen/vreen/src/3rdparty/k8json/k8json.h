/* coded by Ketmar // Vampire Avalon (psyc://ketmar.no-ip.org/~Ketmar)
 * Understanding is not required. Only obedience.
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifndef K8JSON_H
#define K8JSON_H

//#define K8JSON_INCLUDE_GENERATOR
//#define K8JSON_INCLUDE_COMPLEX_GENERATOR


#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

#if defined(K8JSON_INCLUDE_COMPLEX_GENERATOR) || defined(K8JSON_INCLUDE_GENERATOR)
# include <QByteArray>
#endif

#if defined(K8JSON_LIB_MAKEDLL)
#  define K8JSON_EXPORT Q_DECL_EXPORT
#elif defined(K8JSON_LIB_DLL)
#  define K8JSON_EXPORT Q_DECL_IMPORT
#else
#  define K8JSON_EXPORT
#endif


namespace K8JSON {

/*
 * quote string to JSON-friendly format, add '"'
 */
K8JSON_EXPORT QString quote (const QString &str);

/*
 * check if given (const uchar *) represents valid UTF-8 sequence
 * NULL (or empty) s is not valid
 * sequence ends on '\0' if zeroInvalid==false
 */
K8JSON_EXPORT bool isValidUtf8 (const uchar *s, int maxLen, bool zeroInvalid=false);


/*
 * skip blanks and comments
 * return ptr to first non-blank char or 0 on error
 * 'maxLen' will be changed
 */
K8JSON_EXPORT const uchar *skipBlanks (const uchar *s, int *maxLength);

/*
 * skip one record
 * the 'record' is either one full field ( field: val)
 * or one list/object.
 * return ptr to the first non-blank char after the record (or 0)
 * 'maxLen' will be changed
 */
K8JSON_EXPORT const uchar *skipRec (const uchar *s, int *maxLength);

/*
 * parse field value
 * return ptr to the first non-blank char after the value (or 0)
 * 'maxLen' will be changed
 */
K8JSON_EXPORT const uchar *parseValue (QVariant &fvalue, const uchar *s, int *maxLength);


/*
 * parse one field (f-v pair)
 * return ptr to the first non-blank char after the record (or 0)
 * 'maxLen' will be changed
 */
K8JSON_EXPORT const uchar *parseField (QString &fname, QVariant &fvalue, const uchar *s, int *maxLength);

/*
 * parse one record (list or object)
 * return ptr to the first non-blank char after the record (or 0)
 * 'maxLen' will be changed
 */
K8JSON_EXPORT const uchar *parseRecord (QVariant &res, const uchar *s, int *maxLength);


#ifdef K8JSON_INCLUDE_GENERATOR
/*
 * generate JSON text from variant
 * 'err' must be empty (generateEx() will not clear it)
 * return false on error
 */
K8JSON_EXPORT bool generateEx (QString &err, QByteArray &res, const QVariant &val, int indent=0);

/*
 * same as above, but without error message
 */
K8JSON_EXPORT bool generate (QByteArray &res, const QVariant &val, int indent=0);
#endif


#ifdef K8JSON_INCLUDE_COMPLEX_GENERATOR
/*
 * callback for unknown variant type
 * return false and set 'err' on error
 * or return true and *add* converted value (valid sequence of utf-8 bytes) to res
 */
typedef bool (*generatorCB) (void *udata, QString &err, QByteArray &res, const QVariant &val, int indent);

/*
 * generate JSON text from variant
 * 'err' must be empty (generateEx() will not clear it)
 * return false on error
 */
K8JSON_EXPORT bool generateExCB (void *udata, generatorCB cb, QString &err, QByteArray &res, const QVariant &val, int indent=0);

/*
 * same as above, but without error message
 */
K8JSON_EXPORT bool generateCB (void *udata, generatorCB cb, QByteArray &res, const QVariant &val, int indent=0);
#endif


}


#endif
