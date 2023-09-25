#include <QDebug>
#include <QFile>
#include <QTest>

#include "ezsql.h"

sqlite3* db;

class TestStmt : public QObject {
    Q_OBJECT

  private slots:
    void create_db()
    {
        QFile::remove("./test.db");

        int rs = sqlite3_open_v2("./test.db", &db,
                                 SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
                                 nullptr);

        QVERIFY2(rs == SQLITE_OK, "create db failed");
    }

    void create_table()
    {
        EzSql::Stmt stmt(db);

        int rs = stmt.prepare(R"(CREATE TABLE IF NOT EXISTS Test (
                                id INTEGER PRIMARY KEY,
                                text_value TEXT,
                                int_value INTEGER,
                                blod_value BLOB,
                                real_value REAL
                            ))");
        QVERIFY2(rs == SQLITE_OK, "prepare failed");

        rs = stmt.step();
        QVERIFY2(rs == SQLITE_DONE, "step failed");

        rs = stmt.finalize();
        QVERIFY2(rs == SQLITE_OK, "finalize failed");
    }

    void insert()
    {
        EzSql::Stmt stmt(db);
        QVERIFY(stmt.prepare(R"(insert into
                                    Test (
                                        id,
                                        text_value,
                                        int_value,
                                        blod_value,
                                        real_value
                                    )
                                values
                                    (
                                        :id,
                                        :text_value,
                                        :int_value,
                                        :blod_value,
                                        :real_value
                                    ))") == SQLITE_OK);
        QVERIFY(stmt.bind(":id", 1) == SQLITE_OK);
        QVERIFY(stmt.bind(":text_value", QString("test value")) == SQLITE_OK);
        QVERIFY(stmt.bind(":int_value", 13) == SQLITE_OK);
        QVERIFY(stmt.bind(":blod_value", QByteArray("test data")) == SQLITE_OK);
        QVERIFY(stmt.bind(":real_value", 0.13) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_DONE);
        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void select()
    {
        EzSql::Stmt stmt(db);
        QVERIFY(stmt.prepare("select * from Test where id = :id") == SQLITE_OK);
        QVERIFY(stmt.bind(":id", 1) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_ROW);
        QVERIFY(stmt.step() == SQLITE_DONE);
        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void close_db()
    {
        QVERIFY2(sqlite3_close_v2(db) == SQLITE_OK, "close db failed");
        QFile::remove("./test.db");
    }
};

QTEST_MAIN(TestStmt)
#include "test_stmt.moc"
