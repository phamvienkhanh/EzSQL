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
    VALIDATE_DB_STMT

    return sqlite3_bind_blob(_stmt, idx, value, value.size(), SQLITE_TRANSIENT);
}

int Stmt::bind(int idx, const QString& value)
{
    VALIDATE_DB_STMT

    return sqlite3_bind_text(_stmt, idx, value.toUtf8(), value.toUtf8().size(), SQLITE_TRANSIENT);
}

int Stmt::bind(int idx, qint32 value)
{
    VALIDATE_DB_STMT

    return sqlite3_bind_int(_stmt, idx, value);
}

int Stmt::bind(int idx, qint64 value)
{
    VALIDATE_DB_STMT

    return sqlite3_bind_int64(_stmt, idx, value);
}

int Stmt::bind(int idx, double value)
{
    VALIDATE_DB_STMT

    return sqlite3_bind_double(_stmt, idx, value);
}

int Stmt::step()
{
    VALIDATE_DB_STMT

    return sqlite3_step(_stmt);
}

Result Stmt::result() { return Result(_stmt); }

int Stmt::finalize()
{
    VALIDATE_DB_STMT

    return sqlite3_finalize(_stmt);
}

int Stmt::reset()
{
    VALIDATE_DB_STMT

    return sqlite3_reset(_stmt);
}

// implement class Result
Result::Result(sqlite3_stmt* stmt)
{
    _stmt = stmt;

    if (_stmt) {
        _count = sqlite3_column_count(_stmt);
        for (auto i = 0; i < _count; i++) {
            QString colName(sqlite3_column_name(_stmt, i));
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

bool DataBase::open(const OpenParams& params)
{
    if (params.fileName.isEmpty() || !params.flags) {
        return false;
    }

    if (sqlite3_open_v2(params.fileName.toUtf8(), &_db, params.flags, nullptr) != SQLITE_OK) {
        _db = nullptr;
        return false;
    }

    sqlite3_preupdate_hook(

        _db,
        [](void* pCtx,          /* Copy of third arg to preupdate_hook() */
           sqlite3* db,         /* Database handle */
           int op,              /* SQLITE_UPDATE, DELETE or INSERT */
           char const* zDb,     /* Database name */
           char const* zName,   /* Table name */
           sqlite3_int64 iKey1, /* Rowid of row about to be deleted/updated */
           sqlite3_int64 iKey2) {
            qDebug() << "=====================================";
            qDebug() << "name table " << zName;
            qDebug() << "name db " << zDb;
            qDebug() << "op " << op;
            qDebug() << "iKey1 " << iKey1;
            qDebug() << "iKey2 " << iKey2;
            qDebug() << "=====================================";
        },
        nullptr);

    return true;
}

void DataBase::close()
{
    if (_db) {
        sqlite3_close_v2(_db);
        _db = nullptr;
    }
}

}  // namespace EzSql
