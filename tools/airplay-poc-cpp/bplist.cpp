/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "bplist.h"

#include <QList>
#include <QtEndian>
#include <cstring>

namespace airplay {
namespace bplist {

const char kContentType[] = "application/x-apple-binary-plist";

namespace {

const char kMagic[] = "bplist00";
const int kTrailerSize = 32;
const int kRefSize = 2;
const int kOffsetSize = 4;
const int kMaxDepth = 32;

QByteArray BigEndian(quint64 value, int size) {
  QByteArray out(size, '\0');
  for (int i = size - 1; i >= 0; --i) {
    out[i] = char(value & 0xFF);
    value >>= 8;
  }
  return out;
}

QByteArray Integer(qint64 value) {
  if (value >= 0 && value <= 0xFF) return '\x10' + BigEndian(value, 1);
  if (value >= 0 && value <= 0xFFFF) return '\x11' + BigEndian(value, 2);
  if (value >= 0 && value <= 0xFFFFFFFFLL) return '\x12' + BigEndian(value, 4);
  return '\x13' + BigEndian(quint64(value), 8);
}

// The type nibble and count, with the count in a following integer if it
// doesn't fit in four bits.
QByteArray Marker(int type, int count) {
  if (count < 15) return QByteArray(1, char((type << 4) | count));
  return char((type << 4) | 0xF) + Integer(count);
}

class Writer {
 public:
  QByteArray Write(const QVariant& root) {
    Add(root);

    QByteArray out(kMagic);
    QList<quint64> offsets;
    for (const QByteArray& object : objects_) {
      offsets << out.size();
      out += object;
    }
    const quint64 table = out.size();
    for (quint64 offset : offsets) out += BigEndian(offset, kOffsetSize);

    out += QByteArray(6, '\0');
    out += char(kOffsetSize);
    out += char(kRefSize);
    out += BigEndian(objects_.size(), 8);
    out += BigEndian(0, 8);  // The root is the first object.
    out += BigEndian(table, 8);
    return out;
  }

 private:
  int Add(const QVariant& value) {
    const int index = objects_.size();
    objects_ << QByteArray();

    QByteArray object;
    switch (value.typeId()) {
      case QMetaType::Bool:
        object = QByteArray(1, value.toBool() ? '\x09' : '\x08');
        break;
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Short:
      case QMetaType::UShort:
        object = Integer(value.toLongLong());
        break;
      case QMetaType::Double:
      case QMetaType::Float: {
        const double d = value.toDouble();
        quint64 bits;
        std::memcpy(&bits, &d, sizeof(bits));
        object = '\x23' + BigEndian(bits, 8);
        break;
      }
      case QMetaType::QByteArray: {
        const QByteArray data = value.toByteArray();
        object = Marker(0x4, data.size()) + data;
        break;
      }
      case QMetaType::QVariantList: {
        const QVariantList list = value.toList();
        QByteArray refs;
        for (const QVariant& item : list)
          refs += BigEndian(Add(item), kRefSize);
        object = Marker(0xA, list.size()) + refs;
        break;
      }
      case QMetaType::QVariantMap: {
        const QVariantMap map = value.toMap();
        QByteArray keys;
        QByteArray values;
        for (auto it = map.begin(); it != map.end(); ++it) {
          keys += BigEndian(Add(it.key()), kRefSize);
        }
        for (auto it = map.begin(); it != map.end(); ++it) {
          values += BigEndian(Add(it.value()), kRefSize);
        }
        object = Marker(0xD, map.size()) + keys + values;
        break;
      }
      default: {
        const QString s = value.toString();
        bool ascii = true;
        for (QChar c : s) ascii = ascii && c.unicode() < 0x80;
        if (ascii) {
          object = Marker(0x5, s.size()) + s.toLatin1();
        } else {
          object = Marker(0x6, s.size());
          for (QChar c : s) object += BigEndian(c.unicode(), 2);
        }
        break;
      }
    }
    objects_[index] = object;
    return index;
  }

  QList<QByteArray> objects_;
};

class Reader {
 public:
  explicit Reader(const QByteArray& data) : data_(data) {}

  bool Read(QVariant* root) {
    if (data_.size() < int(sizeof(kMagic) - 1) + kTrailerSize ||
        !data_.startsWith(kMagic)) {
      return false;
    }
    end_ = data_.size() - kTrailerSize;
    offset_size_ = quint8(data_[end_ + 6]);
    ref_size_ = quint8(data_[end_ + 7]);
    if (offset_size_ < 1 || offset_size_ > 8 || ref_size_ < 1 ||
        ref_size_ > 8) {
      return false;
    }
    quint64 top;
    if (!UInt(end_ + 8, 8, &count_) || !UInt(end_ + 16, 8, &top) ||
        !UInt(end_ + 24, 8, &table_)) {
      return false;
    }
    if (count_ > quint64(end_) || table_ > quint64(end_) ||
        table_ + count_ * offset_size_ > quint64(end_)) {
      return false;
    }
    return Object(top, 0, root);
  }

 private:
  bool UInt(quint64 pos, int size, quint64* value) const {
    if (pos + size > quint64(data_.size())) return false;
    *value = 0;
    for (int i = 0; i < size; ++i) {
      *value = (*value << 8) | quint8(data_[int(pos) + i]);
    }
    return true;
  }

  // Where an object's contents start and how many items or bytes it has.
  bool Count(quint64 pos, int info, quint64* count, quint64* start) const {
    if (info != 0xF) {
      *count = info;
      *start = pos + 1;
      return true;
    }
    if (pos + 1 >= quint64(end_)) return false;
    const quint8 marker = quint8(data_[int(pos) + 1]);
    if ((marker >> 4) != 0x1 || (marker & 0xF) > 3) return false;
    const int size = 1 << (marker & 0xF);
    *start = pos + 2 + size;
    return UInt(pos + 2, size, count);
  }

  bool Object(quint64 index, int depth, QVariant* out) const {
    if (index >= count_ || depth > kMaxDepth) return false;
    quint64 pos;
    if (!UInt(table_ + index * offset_size_, offset_size_, &pos) ||
        pos >= quint64(end_)) {
      return false;
    }
    const quint8 marker = quint8(data_[int(pos)]);
    const int type = marker >> 4;
    const int info = marker & 0xF;

    switch (type) {
      case 0x0:
        if (info == 0x8 || info == 0x9) {
          *out = info == 0x9;
          return true;
        }
        if (info == 0x0) {
          *out = QVariant();
          return true;
        }
        return false;
      case 0x1: {
        const int size = 1 << info;
        if (size > 16) return false;
        quint64 value;
        // A 16-byte integer is only used for values that fit in 8 bytes.
        if (!UInt(pos + 1 + (size == 16 ? 8 : 0), qMin(size, 8), &value)) {
          return false;
        }
        *out = qint64(value);
        return true;
      }
      case 0x2:
      case 0x3: {
        const int size = 1 << info;
        quint64 bits;
        if ((size != 4 && size != 8) || !UInt(pos + 1, size, &bits)) {
          return false;
        }
        if (size == 4) {
          const quint32 bits32 = quint32(bits);
          float f;
          std::memcpy(&f, &bits32, sizeof(f));
          *out = double(f);
        } else {
          double d;
          std::memcpy(&d, &bits, sizeof(d));
          *out = d;
        }
        return true;
      }
      case 0x4:
      case 0x5:
      case 0x6: {
        quint64 count, start;
        if (!Count(pos, info, &count, &start)) return false;
        const quint64 bytes = type == 0x6 ? count * 2 : count;
        if (start + bytes > quint64(end_)) return false;
        const QByteArray raw = data_.mid(int(start), int(bytes));
        if (type == 0x4) {
          *out = raw;
        } else if (type == 0x5) {
          *out = QString::fromLatin1(raw);
        } else {
          QString s;
          for (int i = 0; i + 1 < raw.size(); i += 2) {
            s += QChar(qFromBigEndian<quint16>(raw.constData() + i));
          }
          *out = s;
        }
        return true;
      }
      case 0x8: {
        quint64 value;
        if (!UInt(pos + 1, info + 1, &value)) return false;
        *out = qint64(value);
        return true;
      }
      case 0xA:
      case 0xD: {
        quint64 count, start;
        if (!Count(pos, info, &count, &start)) return false;
        const quint64 refs = type == 0xD ? count * 2 : count;
        if (start + refs * ref_size_ > quint64(end_)) return false;
        QList<QVariant> items;
        for (quint64 i = 0; i < refs; ++i) {
          quint64 ref;
          QVariant item;
          if (!UInt(start + i * ref_size_, ref_size_, &ref) ||
              !Object(ref, depth + 1, &item)) {
            return false;
          }
          items << item;
        }
        if (type == 0xA) {
          *out = QVariantList(items);
        } else {
          QVariantMap map;
          for (quint64 i = 0; i < count; ++i) {
            map.insert(items[i].toString(), items[count + i]);
          }
          *out = map;
        }
        return true;
      }
      default:
        return false;
    }
  }

  const QByteArray& data_;
  int end_ = 0;
  int offset_size_ = 0;
  int ref_size_ = 0;
  quint64 count_ = 0;
  quint64 table_ = 0;
};

}  // namespace

QByteArray Write(const QVariant& root) { return Writer().Write(root); }

bool Read(const QByteArray& data, QVariant* root) {
  return Reader(data).Read(root);
}

}  // namespace bplist
}  // namespace airplay
