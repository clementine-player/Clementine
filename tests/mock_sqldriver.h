#include <gmock/gmock.h>

#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlIndex>
#include <QSqlField>

class MockSqlDriver : public QSqlDriver {
 public:
  virtual ~MockSqlDriver() {}
  MOCK_METHOD0(beginTransaction, bool());
  MOCK_METHOD0(close, void());
  MOCK_METHOD0(commitTransaction, bool());
  MOCK_CONST_METHOD0(createResult, QSqlResult*());
  MOCK_CONST_METHOD2(escapeIdentifier, QString(const QString&, IdentifierType));
  MOCK_CONST_METHOD2(formatValue, QString(const QSqlField&, bool));
  MOCK_CONST_METHOD0(handle, QVariant());
  MOCK_CONST_METHOD1(hasFeature, bool(DriverFeature));
  MOCK_CONST_METHOD0(isOpen, bool());
  MOCK_METHOD6(open, bool(const QString&, const QString&, const QString&, const QString&, int, const QString&));
  MOCK_CONST_METHOD1(primaryIndex, QSqlIndex(const QString&));
  MOCK_CONST_METHOD1(record, QSqlRecord(const QString&));
  MOCK_METHOD0(rollbackTransaction, bool());
  MOCK_CONST_METHOD4(sqlStatement, QString(StatementType, const QString&, const QSqlRecord&, bool));
  MOCK_CONST_METHOD1(tables, QStringList(QSql::TableType));

  MOCK_METHOD1(setLastError, void(const QSqlError&));
  MOCK_METHOD1(setOpen, void(bool));
  MOCK_METHOD1(setOpenError, void(bool));
};


class MockSqlResult : public QSqlResult {
 public:
  MockSqlResult(QSqlDriver* driver)
      : QSqlResult(driver) {
  }
  virtual ~MockSqlResult() {}
  MOCK_METHOD3(bindValue, void(int, const QVariant&, QSql::ParamType));
  MOCK_METHOD3(bindValue, void(const QString&, const QVariant&, QSql::ParamType));
  MOCK_METHOD1(data, QVariant(int));
  MOCK_METHOD0(exec, bool());
  MOCK_METHOD1(fetch, bool(int));
  MOCK_METHOD0(fetchFirst, bool());
  MOCK_METHOD0(fetchLast, bool());
  MOCK_METHOD1(isNull, bool(int));
  MOCK_CONST_METHOD0(lastInsertId, QVariant());
  MOCK_METHOD0(numRowsAffected, int());
  MOCK_METHOD1(prepare, bool(const QString&));
  MOCK_METHOD1(reset, bool(const QString&));
  MOCK_METHOD0(size, int());

  void hackSetActive(const QString&) {
    setActive(true);
    setAt(0);
    setSelect(true);
  }

  void hackSetActiveVoid() {
    hackSetActive(QString());
  }

  void setAt(int i) {
    QSqlResult::setAt(i);
  }
};
