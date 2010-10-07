/* This file is part of Clementine.

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

#include "lastfmstationdialog.h"
#include "ui_lastfmstationdialog.h"

#include <map>
#include <string>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <QtDebug>
#include <QValidator>

namespace {

namespace qi = boost::spirit::qi;
using boost::spirit::lit;

template <typename Iterator>
struct rql : qi::grammar<Iterator, std::vector<std::pair<std::string, std::string> >()> {
  rql() : rql::base_type(query) {
    query = pair % lit(' ') >> qi::eoi;
    pair = key >> lit(':') >> value;
    key = qi::string("simart") |
          qi::string("tag") |
          qi::string("user") |
          qi::string("library") |
          qi::string("loved");
    value = (unquoted | quoted);
    unquoted = +(qi::char_ - lit(' ') - lit('"'));
    quoted = '"' >> +(qi::char_ - '"') >> '"';
  }

  qi::rule<Iterator, std::vector<std::pair<std::string, std::string> >()> query;
  qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
  qi::rule<Iterator, std::string()> key;
  qi::rule<Iterator, std::string()> value, unquoted, quoted;
};

class RQLValidator : public QValidator {
 public:
  QValidator::State validate(QString& str, int& pos) const {
    std::string input = str.toStdString();
    rql<std::string::iterator> parser;
    std::vector<std::pair<std::string, std::string> > results;
    bool ret = qi::parse(input.begin(), input.end(), parser, results);

    qDebug() << "Success:" << ret;

    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = results.begin();
        it != results.end(); ++it) {
      qDebug("key:%s value:%s", it->first.c_str(), it->second.c_str());
    }
    qDebug() << "---";

    return ret ? Acceptable : Intermediate;
  }
};

}  // namespace

LastFMStationDialog::LastFMStationDialog(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_LastFMStationDialog)
{
  ui_->setupUi(this);

  resize(sizeHint());

  ui_->content->setValidator(new RQLValidator());
}

LastFMStationDialog::~LastFMStationDialog() {
  delete ui_;
}

void LastFMStationDialog::SetType(Type type) {
  ui_->type->setCurrentIndex(type);
  ui_->content->clear();
  ui_->content->setFocus(Qt::OtherFocusReason);
}

QString LastFMStationDialog::content() const {
  return ui_->content->text();
}

void LastFMStationDialog::accept() {
}
