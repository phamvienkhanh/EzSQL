#include <QDebug>
#include <QFile>
#include <QTest>

#include "ezsql.h"

sqlite3* db;

class TestResult : public QObject {
    Q_OBJECT

  private slots:
    void create_db()
    {
        QFile::remove("./test_result.db");

        int rs = sqlite3_open_v2("./test_result.db", &db,
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

    void result1()
    {
        EzSql::Stmt stmt(db);
        QVERIFY(stmt.prepare("select * from Test where id = :id") == SQLITE_OK);
        QVERIFY(stmt.bind(":id", 1) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_ROW);

        auto result = stmt.result();

        qint32 id;
        QVERIFY(result.map(id, "id"));
        QVERIFY(id == 1);

        QString text_value;
        QVERIFY(result.map(text_value, "text_value"));
        QVERIFY(text_value == QString("test value"));

        QByteArray blod_value;
        QVERIFY(result.map(blod_value, "blod_value"));
        QVERIFY(blod_value == QByteArray("test data"));

        qint32 int_value;
        QVERIFY(result.map(int_value, "int_value"));
        QVERIFY(int_value == 13);

        bool bool_value;
        QVERIFY(result.map(bool_value, "int_value"));
        QVERIFY(bool_value == true);

        double real_value;
        QVERIFY(result.map(real_value, "real_value"));
        QVERIFY(real_value == 0.13);

        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void result2()
    {
        EzSql::Stmt stmt(db);
        QVERIFY(stmt.prepare("select count(*) from Test") == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_ROW);

        auto result = stmt.result();
        qint32 numrow;
        QVERIFY(result.map(numrow, "count(*)"));
        QVERIFY(numrow == 1);

        QVERIFY(result.map(numrow, 0));
        QVERIFY(numrow == 1);

        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void close_db()
    {
        QVERIFY2(sqlite3_close_v2(db) == SQLITE_OK, "close db failed");
        QFile::remove("./test_result.db");
    }
};

QTEST_MAIN(TestResult)
#include "test_result.moc"