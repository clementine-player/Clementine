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

#include "playlistfilterparser.h"
#include "playlist.h"

#include <QAbstractItemModel>

class SearchTermComparator {
 public:
  virtual ~SearchTermComparator() {}
  virtual bool Matches(const QString& element) const = 0;
};

// "compares" by checking if the field contains the search term
class DefaultComparator : public SearchTermComparator {
 public:
  explicit DefaultComparator(const QString& value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element.contains(search_term_);
  }
 private:
  QString search_term_;
};

class EqComparator : public SearchTermComparator {
 public:
  explicit EqComparator(const QString& value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return search_term_ == element;
  }
 private:
  QString search_term_;
};

class LexicalGtComparator : public SearchTermComparator {
 public:
  explicit LexicalGtComparator(const QString& value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element > search_term_;
  }
 private:
  QString search_term_;
};

class LexicalGeComparator : public SearchTermComparator {
 public:
  explicit LexicalGeComparator(const QString& value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element >= search_term_;
  }
 private:
  QString search_term_;
};

class LexicalLtComparator : public SearchTermComparator {
 public:
  explicit LexicalLtComparator(const QString& value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element < search_term_;
  }
 private:
  QString search_term_;
};

class LexicalLeComparator : public SearchTermComparator {
 public:
  explicit LexicalLeComparator(const QString& value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element <= search_term_;
  }
 private:
  QString search_term_;
};

class GtComparator : public SearchTermComparator {
 public:
  explicit GtComparator(int value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element.toInt() > search_term_;
  }
 private:
  int search_term_;
};

class GeComparator : public SearchTermComparator {
 public:
  explicit GeComparator(int value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element.toInt() >= search_term_;
  }
 private:
  int search_term_;
};

class LtComparator : public SearchTermComparator {
 public:
  explicit LtComparator(int value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element.toInt() < search_term_;
  }
 private:
  int search_term_;
};

class LeComparator : public SearchTermComparator {
 public:
  explicit LeComparator(int value) : search_term_(value) {}
  virtual bool Matches(const QString& element) const {
    return element.toInt() <= search_term_;
  }
 private:
  int search_term_;
};

// The length field of the playlist (entries) contains a
// song's running time in nano seconds. However, We don't
// really care about nano seconds, just seconds. Thus, with
// this decorator we drop the last 9 digits, if that many
// are present.
class DropTailComparatorDecorator : public SearchTermComparator {
 public:
  explicit DropTailComparatorDecorator(SearchTermComparator* cmp) : cmp_(cmp) {}

  virtual bool Matches(const QString& element) const {
    if (element.length() > 9)
      return cmp_->Matches(element.left(element.length()-9));
    else
      return cmp_->Matches(element);
  }
 private:
  QScopedPointer<SearchTermComparator> cmp_;
};

// filter that applies a SearchTermComparator to all fields of a playlist entry
class FilterTerm : public FilterTree {
 public:
  explicit FilterTerm(SearchTermComparator* comparator, const QList<int>& columns) : cmp_(comparator), columns_(columns) {}

  virtual bool accept(int row, const QModelIndex& parent, const QAbstractItemModel* const model) const {
    foreach (int i, columns_) {
      QModelIndex idx(model->index(row, i, parent));
      if (cmp_->Matches(idx.data().toString().toLower()))
        return true;
    }
    return false;
  }
  virtual FilterType type() { return Term; }
 private:
  QScopedPointer<SearchTermComparator> cmp_;
  QList<int> columns_;
};

// filter that applies a SearchTermComparator to one specific field of a playlist entry
class FilterColumnTerm : public FilterTree {
 public:
  FilterColumnTerm(int column, SearchTermComparator* comparator) : col(column), cmp_(comparator) {}

  virtual bool accept(int row, const QModelIndex& parent, const QAbstractItemModel* const model) const {
    QModelIndex idx(model->index(row, col, parent));
    return cmp_->Matches(idx.data().toString().toLower());
  }
  virtual FilterType type() { return Column; }
 private:
  int col;
  QScopedPointer<SearchTermComparator> cmp_;
};

class NotFilter : public FilterTree {
 public:
  explicit NotFilter(const FilterTree* inv) : child_(inv) {}
  virtual ~NotFilter() { delete child_; }
  virtual bool accept(int row, const QModelIndex& parent, const QAbstractItemModel* const model) const {
    return !child_->accept(row, parent, model);
  }
  virtual FilterType type() { return Not; }
 private:
  const FilterTree* child_;
};

class OrFilter : public FilterTree  {
 public:
  ~OrFilter() { qDeleteAll(children_); }
  virtual void add(FilterTree* child) { children_.append(child); }
  virtual bool accept(int row, const QModelIndex& parent, const QAbstractItemModel* const model) const {
    foreach (FilterTree* child, children_) {
      if (child->accept(row, parent, model))
        return true;
    }
    return false;
  }
  FilterType type() { return Or; }
 private:
  QList<FilterTree*> children_;
};

class AndFilter : public FilterTree {
 public:
  virtual ~AndFilter() { qDeleteAll(children_); }
  virtual void add(FilterTree* child) { children_.append(child); }
  virtual bool accept(int row, const QModelIndex& parent, const QAbstractItemModel* const model) const {
    foreach (FilterTree* child, children_) {
      if (!child->accept(row, parent, model))
        return false;
    }
    return true;
  }
  FilterType type() { return And; }
 private:
  QList<FilterTree*> children_;
};

FilterParser::FilterParser(const QString& filter, const QMap< QString, int >& columns, QSet< int > exact_cols)
    : filterstring_(filter), columns_(columns), exact_columns_(exact_cols)
{
}

FilterTree* FilterParser::parse() {
  iter_ = filterstring_.constBegin();
  end_ = filterstring_.constEnd();
  return parseOrGroup();
}

void FilterParser::advance() {
  while (iter_ != end_ && iter_->isSpace()) {
    ++iter_;
  }
}

FilterTree* FilterParser::parseOrGroup() {
  advance();
  if (iter_ == end_)
    return new NopFilter;

  OrFilter* group = new OrFilter;
  group->add(parseAndGroup());
  advance();
  while (checkOr()) {
    group->add(parseAndGroup());
    advance();
  }
  return group;
}

FilterTree* FilterParser::parseAndGroup() {
  advance();
  if (iter_ == end_)
    return new NopFilter;

  AndFilter* group = new AndFilter();
  do {
    group->add(parseSearchExpression());
    advance();
    if (iter_ != end_ && *iter_ == QChar(')'))
      break;
    if (checkOr(false)) {
      break;
    }
    checkAnd();  // if there's no 'AND', we'll add the term anyway...
  } while (iter_ != end_);
  return group;
}

bool FilterParser::checkAnd() {
  if (iter_ != end_) {
    if (*iter_ == QChar('A')) {
      buf_ += *iter_;
      iter_++;
      if (iter_ != end_ && *iter_ == QChar('N')) {
        buf_ += *iter_;
        iter_++;
        if (iter_ != end_ && *iter_ == QChar('D')) {
          buf_ += *iter_;
          iter_++;
          if (iter_ != end_ && (iter_->isSpace() || *iter_ == QChar('-') || *iter_ == '(')) {
            advance();
            buf_.clear();
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool FilterParser::checkOr(bool step_over) {
  if (!buf_.isEmpty()) {
    if (buf_ == QString("OR")) {
      if (step_over) {
        buf_.clear();
        advance();
      }
      return true;
    }
  } else {
    if (iter_ != end_) {
      if (*iter_ == QChar('O')) {
        buf_ += *iter_;
        iter_++;
        if (iter_ != end_ && *iter_ == QChar('R')) {
          buf_ += *iter_;
          iter_++;
          if (iter_ != end_ && (iter_->isSpace() || *iter_ == QChar('-') || *iter_ == '(')) {
            if (step_over) {
              buf_.clear();
              advance();
            }
            return true;
          }
        }
      }
    }
  }
  return false;
}

FilterTree* FilterParser::parseSearchExpression() {
  advance();
  if (iter_ == end_)
    return new NopFilter;
  if (*iter_ == QChar('(')) {
    iter_++;
    advance();
    FilterTree* tree = parseOrGroup();
    advance();
    if (iter_ != end_) {
      if (*iter_ == QChar(')')) {
        ++iter_;
      }
    }
    return tree;
  } else if (*iter_ == QChar('-')) {
    ++iter_;
    FilterTree* tree = parseSearchExpression();
    if (tree->type() != FilterTree::Nop)
      return new NotFilter(tree);
    return tree;
  } else {
    return parseSearchTerm();
  }
}

FilterTree* FilterParser::parseSearchTerm() {
  QString col;
  QString search;
  QString prefix;
  bool inQuotes = false;
  for (; iter_ != end_; ++iter_) {
    if (inQuotes) {
      if (*iter_ == QChar('"'))
        inQuotes = false;
      else
        buf_ += *iter_;
    } else {
      if (*iter_ == QChar('"')) {
        inQuotes = true;
      } else if (col.isEmpty() && *iter_ == QChar(':')) {
        col = buf_.toLower();
        buf_.clear();
        prefix.clear();  // prefix isn't allowed here - let's ignore it
      } else if (iter_->isSpace() ||
                  *iter_ == QChar('(') ||
                  *iter_ == QChar(')') ||
                  *iter_ == QChar('-')) {
          break;
      } else if (buf_.isEmpty()) {
        // we don't know whether there is a column part in this search term
        // thus we assume the latter and just try and read a prefix
        if (prefix.isEmpty() &&
            (*iter_ == QChar('>') || *iter_ == QChar('<') || *iter_ == QChar('='))) {
          prefix += *iter_;
        } else if (prefix != QString("=") && *iter_ == QChar('=')) {
          prefix += *iter_;
        } else {
          buf_ += *iter_;
        }
      } else {
        buf_ += *iter_;
      }
    }
  }

  search = buf_.toLower();
  buf_.clear();

  return createSearchTermTreeNode(col, prefix, search);
}

FilterTree* FilterParser::createSearchTermTreeNode(const QString& col, const QString& prefix, const QString& search) const {
  if (search.isEmpty())
    return new NopFilter;
  if (prefix.isEmpty()) {
    // easy case - no prefix :-)
    if (!col.isEmpty() && columns_.contains(col))
      return new FilterColumnTerm(columns_[col], exact_columns_.contains(columns_[col]) ?
          static_cast<SearchTermComparator*>(new EqComparator(search)) :
          static_cast<SearchTermComparator*>(new DefaultComparator(search)) );
    else
      return new FilterTerm(new DefaultComparator(search), columns_.values());
  } else {
    // here comes a mess :/
    // well, not that much of a mess, but so many options -_-
    SearchTermComparator* cmp = NULL;
    if (prefix == QString("=")) {
      cmp = new EqComparator(search);
    } else if (!col.isEmpty() && columns_.contains(col) && exact_columns_.contains(columns_[col])) {
      // the length column contains the time in seconds (nano seconds, actually -
      //  the "nano" part is handled by the DropTailComparatorDecorator,  though).
      QString _search = columns_[col] == Playlist::Column_Length ? parseTime(search) : search;

      // alright, back to deciding which comparator we'll use
      if (prefix == QString(">")) {
        cmp = new GtComparator(_search.toInt());
      } else if (prefix == QString(">=")) {
        cmp = new GeComparator(_search.toInt());
      } else if (prefix == QString("<")) {
        cmp = new LtComparator(_search.toInt());
      } else if (prefix == QString("<=")) {
        cmp = new LeComparator(_search.toInt());
      } else {
        // just to be sure :-)
        cmp = new EqComparator(_search);
      }
    } else {
      if (prefix == QString(">")) {
        cmp = new LexicalGtComparator(search);
      } else if (prefix == QString(">=")) {
        cmp = new LexicalGeComparator(search);
      } else if (prefix == QString("<")) {
        cmp = new LexicalLtComparator(search);
      } else if (prefix == QString("<=")) {
        cmp = new LexicalLeComparator(search);
      } else {
        // just to be sure :-)
        cmp = new DefaultComparator(search);
      }
    }
    if (columns_.contains(col) && columns_[col] == Playlist::Column_Length)
      cmp = new DropTailComparatorDecorator(cmp);
    if (columns_.contains(col))
      return new FilterColumnTerm(columns_[col], cmp);
    else
      return new FilterTerm(cmp, columns_.values());
  }
}

// Try and parse the string as '[[h:]m:]s' (ignoring all spaces),
// and return the number of seconds if it parses correctly.
// If not, the original string is returned.
// The 'h', 'm' and 's' components can have any length (including 0).
//
// A few examples:
//  "::"       is parsed to "0"
//  "1::"      is parsed to "3600"
//  "3:45"     is parsed to "225"
//  "1:165"    is parsed to "225"
//  "225"      is parsed to "225" (srsly! ^.^)
//  "2:3:4:5"  is parsed to "2:3:4:5"
//  "25m"      is parsed to "25m"
QString FilterParser::parseTime(const QString& timeStr) const {
  int seconds = 0, accum = 0, colonCount = 0;
  foreach (const QChar& c, timeStr) {
    if (c.isDigit()) {
      accum = accum*10+c.digitValue();
    } else if (c == ':') {
      seconds = seconds*60+accum;
      accum = 0;
      ++colonCount;
      if (colonCount>2)
        return timeStr;
    } else if (!c.isSpace()) {
      return timeStr;
    }
  }
  seconds = seconds*60+accum;
  return QString::number(seconds);
}

