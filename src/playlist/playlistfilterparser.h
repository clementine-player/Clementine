/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYLISTFILTERPARSER_H
#define PLAYLISTFILTERPARSER_H

#include <QMap>
#include <QModelIndex>
#include <QSet>
#include <QString>

class QAbstractItemModel;

// structure for filter parse tree
class FilterTree {
 public:
  virtual ~FilterTree() {}
  virtual bool accept(int row, const QModelIndex& parent,
                      const QAbstractItemModel* const model) const = 0;
  enum FilterType { Nop = 0, Or, And, Not, Column, Term };
  virtual FilterType type() = 0;
};

// trivial filter that accepts *anything*
class NopFilter : public FilterTree {
 public:
  virtual bool accept(int row, const QModelIndex& parent,
                      const QAbstractItemModel* const model) const {
    return true;
  }
  virtual FilterType type() { return Nop; }
};

// A utility class to parse search filter strings into a decision tree
// that can decide whether a playlist entry matches the filter.
//
// Here's a grammar describing the filters we expect:
//   　expr      ::= or-group
//     or-group  ::= and-group ('OR' and-group)*
//     and-group ::= sexpr ('AND' sexpr)*
//     sexpr     ::= sterm | '-' sexpr | '(' or-group ')'
//     sterm     ::= col ':' sstring | sstring
//     sstring   ::= prefix? string
//     string    ::= [^:-()" ]+ | '"' [^"]+ '"'
//     prefix    ::= '=' | '<' | '>' | '<=' | '>='
//     col       ::= "title" | "artist" | ...
class FilterParser {
 public:
  FilterParser(const QString& filter, const QMap<QString, int>& columns,
               const QSet<int>& numerical_cols);

  FilterTree* parse();

 private:
  void advance();
  FilterTree* parseOrGroup();
  FilterTree* parseAndGroup();
  // check if iter is at the start of 'AND'
  // if so, step over it and return true
  // it not, return false and leave iter where it was
  bool checkAnd();
  // check if iter is at the start of 'OR'
  bool checkOr(bool step_over = true);
  FilterTree* parseSearchExpression();
  FilterTree* parseSearchTerm();

  FilterTree* createSearchTermTreeNode(const QString& col,
                                       const QString& prefix,
                                       const QString& search) const;
  int parseTime(const QString& time_str) const;

  QString::const_iterator iter_;
  QString::const_iterator end_;
  QString buf_;
  const QString filterstring_;
  const QMap<QString, int> columns_;
  const QSet<int> numerical_columns_;
};

#endif  // PLAYLISTFILTERPARSER_H
