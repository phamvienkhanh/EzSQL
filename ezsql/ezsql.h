#ifndef EZSQL_H
#define EZSQL_H

#include <sqlite3.h>

#include <QMap>
#include <QString>

namespace EzSql {

class BaseDBO {
private:
  /* data */
public:
  BaseDBO(/* args */) {}
  ~BaseDBO() {}
};

class Result {
public:
  Result(sqlite3_stmt *stmt = nullptr);

  bool map(QString &value, int idx = 0);
  bool map(QByteArray &value, int idx = 0);
  bool map(qint32 &value, int idx = 0);
  bool map(qint64 &value, int idx = 0);
  bool map(double &value, int idx = 0);
  bool map(bool &value, int idx = 0);

  template <typename T> bool map(T &value, const QString &name) {
    if (!_stmt) {
      return false;
    }

    if (!_columns.contains(name)) {
      return false;
    }

    return map(value, _columns[name]);
  }

  template <typename T> bool map(T &value) {
    static_assert(std::is_base_of_v<BaseDBO, T>,
                  "map type T error. T must base of BaseDBO");
    return true;
  }

private:
  QMap<QString, qint32> _columns; // <colname, index>
  sqlite3_stmt *_stmt = nullptr;
};

class Stmt {
public:
  Stmt(sqlite3 *db = nullptr);
  int prepare(const QString &query);

  int bind(int idx, const QByteArray &value);
  int bind(int idx, const QString &value);
  int bind(int idx, qint32 value);
  int bind(int idx, qint64 value);
  int bind(int idx, double value);

  template <typename T> int bind(const QString &name, const T &value) {
    if (!_db || !_stmt) {
      return SQLITE_ERROR;
    }

    int idx = sqlite3_bind_parameter_index(_stmt, name.toUtf8());
    if (!idx) {
      return SQLITE_ERROR;
    }

    return bind(idx, value);
  }

  int step();
  Result result();
  int finalize();
  int reset();

  inline void db(sqlite3 *db) { _db = db; }

private:
  sqlite3_stmt *_stmt = nullptr;
  sqlite3 *_db = nullptr;
};

} // namespace EzSql

#endif // EZSQL_H
