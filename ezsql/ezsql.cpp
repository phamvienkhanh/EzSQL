#include "ezsql.h"

#include <QDebug>

#define VALIDATE_DB_STMT     \
    if (!_db || !_stmt) {    \
        return SQLITE_ERROR; \
    }

#define VALIDATE_STMT \
    if (!_stmt) {     \
        return false; \
    }

#define VALIDATE_IDX                                    \
    if (idx < 0 || idx > sqlite3_column_count(_stmt)) { \
        return false;                                   \
    }

namespace EzSql {

// implement class Stmt
Stmt::Stmt(sqlite3* db) { _db = db; }

int Stmt::prepare(const QString& query)
{
    if (!_db) {
        return SQLITE_ERROR;
    }

    return sqlite3_prepare_v2(_db, query.toUtf8(), query.toUtf8().size(), &_stmt, nullptr);
}

int Stmt::bind(int idx, const QByteArray& value)
{
    if (!_db || !_stmt) {
        return SQLITE_ERROR;
    }

    return sqlite3_bind_blob(_stmt, idx, value, value.size(), SQLITE_TRANSIENT);
}

int Stmt::bind(int idx, const QString& value)
{
    if (!_db || !_stmt) {
        return SQLITE_ERROR;
    }

    return sqlite3_bind_text(_stmt, idx, value.toUtf8(), value.toUtf8().size(), SQLITE_TRANSIENT);
}

int Stmt::bind(int idx, qint32 value)
{
    if (!_db || !_stmt) {
        return SQLITE_ERROR;
    }

    return sqlite3_bind_int(_stmt, idx, value);
}

int Stmt::bind(int idx, qint64 value)
{
    if (!_db || !_stmt) {
        return SQLITE_ERROR;
    }

    return sqlite3_bind_int64(_stmt, idx, value);
}

int Stmt::bind(int idx, double value)
{
    if (!_db || !_stmt) {
        return SQLITE_ERROR;
    }

    return sqlite3_bind_double(_stmt, idx, value);
}

int Stmt::step()
{
    VALIDATE_DB_STMT;

    return sqlite3_step(_stmt);
}

Result Stmt::result() { return Result(_stmt); }

int Stmt::finalize()
{
    VALIDATE_DB_STMT;

    return sqlite3_finalize(_stmt);
}

int Stmt::reset()
{
    VALIDATE_DB_STMT;

    return sqlite3_reset(_stmt);
}

// implement class Result
Result::Result(sqlite3_stmt* stmt)
{
    _stmt = stmt;

    if (_stmt) {
        qint32 cols = sqlite3_column_count(_stmt);
        for (auto i = 0; i < cols; i++) {
            QString colName(sqlite3_column_name(_stmt, i));
            qDebug() << colName;
            _columns.insert(colName, i);
        }
    }
}

bool Result::map(QString& value, int idx /*= 0*/)
{
    VALIDATE_STMT;
    VALIDATE_IDX;

    int sz = sqlite3_column_bytes(_stmt, idx);
    value = QString::fromUtf8((char*)sqlite3_column_text(_stmt, idx), sz);

    return true;
}

bool Result::map(QByteArray& value, int idx /*=0*/)
{
    VALIDATE_STMT;
    VALIDATE_IDX;

    int sz = sqlite3_column_bytes(_stmt, idx);
    value = QByteArray((char*)sqlite3_column_blob(_stmt, idx), sz);

    return true;
}

bool Result::map(qint32& value, int idx /*=0*/)
{
    VALIDATE_STMT;
    VALIDATE_IDX;

    value = sqlite3_column_int(_stmt, idx);

    return true;
}

bool Result::map(qint64& value, int idx /*=0*/)
{
    VALIDATE_STMT;
    VALIDATE_IDX;

    value = sqlite3_column_int64(_stmt, idx);

    return true;
}

bool Result::map(double& value, int idx /*=0*/)
{
    VALIDATE_STMT;
    VALIDATE_IDX;

    value = sqlite3_column_double(_stmt, idx);

    return true;
}

bool Result::map(bool& value, int idx /*=0*/)
{
    VALIDATE_STMT;
    VALIDATE_IDX;

    value = sqlite3_column_int(_stmt, idx);

    return true;
}

}  // namespace EzSql
