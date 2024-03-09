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
};

QTEST_MAIN(TestDB)
#include "test_db.moc"
