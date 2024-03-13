#include <QFile>
#include <QTest>

#include <ezsql.h>

class TestDBO : public EzSql::BaseDBO
{
public:
    qint32 _id;
    QString _text;
    QByteArray _blob;
    bool _boolean;
    double _double;
    qint64 _int64;

public:
    TestDBO() = default;

    void fields() override
    {
        table("TestDBO");

        id("id", &_id, "", "NOT NULL PRIMARY KEY AUTOINCREMENT");
        field("field_text", &_text);
        field("field_blob", &_blob);
        field("field_boolean", &_boolean);
        field("field_double", &_double);
        field("field_int64", &_int64);
    }
};

class TestDB : public QObject
{
    Q_OBJECT

private:
    EzSql::DataBase db;
private slots:

    void create_db()
    {
        QFile::remove("./test_db.db");

        EzSql::DataBase::OpenParams params;
        params.fileName = "./test_db.db";
        params.flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;

        QVERIFY2(db.open(params), "open db failed");
    }

    void createTable() {
        bool rs = db.createTable<TestDBO>();
        QVERIFY2(rs, "create table TestDBO failed");
    }

    void insert_stmt()
    {
        TestDBO a;
        a._int64 = 1233333;
        a._boolean = false;
        a._text = "something data";
        a._double = 0.13;

        EzSql::Stmt stmt(db.connection());
        QVERIFY(stmt.prepare(a.insertStmt()) == SQLITE_OK);
        QVERIFY(a.bind(stmt, 0) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_ROW);
        auto result = stmt.result();
        a.setId(result);
        QVERIFY(a._id == 1);
        QVERIFY(stmt.step() == SQLITE_DONE);

        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void update_stmt()
    {
        TestDBO a;
        a._id = 1;
        a._text = "text updated";
        a._int64 = 5;
        a._boolean = false;
        a._double = 1.0;

        EzSql::Stmt stmt(db.connection());
        QVERIFY(stmt.prepare(a.updateStmt()) == SQLITE_OK);
        QVERIFY(a.bind(stmt, a.allFieldName()) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_DONE);
        QVERIFY(stmt.finalize() == SQLITE_OK);
    }
};

QTEST_MAIN(TestDB)
#include "test_db.moc"
