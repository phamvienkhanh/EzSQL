#include <QDebug>
#include <QFile>
#include <QTest>

#include "ezsql.h"

class TestDBO : public EzSql::BaseDBO {
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

        id("id", &_id);
        field("field_text", &_text);
        field("field_blob", &_blob);
        field("field_boolean", &_boolean);
        field("field_double", &_double);
        field("field_int64", &_int64);
    }
};

sqlite3* db;

class TestBaseDBO : public QObject {
    Q_OBJECT

  private slots:
  void create_db()
    {
        QFile::remove("./test_basedbo.db");

        int rs = sqlite3_open_v2("./test_basedbo.db", &db,
                                 SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
                                 nullptr);

        QVERIFY2(rs == SQLITE_OK, "create db failed");
    }

    void create_table()
    {
        EzSql::Stmt stmt(db);

        int rs = stmt.prepare(R"(CREATE TABLE IF NOT EXISTS Test (
                                id INTEGER PRIMARY KEY,
                                field_text TEXT,
                                field_int64 INTEGER,
                                field_blob BLOB,
                                field_double REAL,
                                feild_boolean INTEGER
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
                                        field_text,
                                        field_int64,
                                        field_blob,
                                        field_double,
                                        feild_boolean
                                    )
                                values
                                    (
                                        :id,
                                        :text_value,
                                        :int_value,
                                        :blod_value,
                                        :real_value,
                                        :boolean_value
                                    ))") == SQLITE_OK);
        QVERIFY(stmt.bind(":id", 1) == SQLITE_OK);
        QVERIFY(stmt.bind(":text_value", QString("test value")) == SQLITE_OK);
        QVERIFY(stmt.bind(":int_value", 13) == SQLITE_OK);
        QVERIFY(stmt.bind(":blod_value", QByteArray("test data")) == SQLITE_OK);
        QVERIFY(stmt.bind(":real_value", 0.13) == SQLITE_OK);
        QVERIFY(stmt.bind(":boolean_value", false) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_DONE);
        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void test_get_set()
    {
        TestDBO a;
        a.fields();

        a.set("id", 13);
        QVERIFY(a.get<int>("id") == 13);

        a.set("field_text", QString("this is a text"));
        QVERIFY(a.get<QString>("field_text") == QString("this is a text"));

        a.set("field_blob", QByteArray("this is a blob"));
        QVERIFY(a.get<QByteArray>("field_blob") == QByteArray("this is a blob"));

        a.set("field_boolean", false);
        QVERIFY(a.get<bool>("field_boolean") == false);

        a.set("field_double", 0.13);
        QVERIFY(a.get<double>("field_double") == 0.13);

        a.set("field_int64", (qint64)123);
        QVERIFY(a.get<qint64>("field_int64") == 123);
    }

    void create_stmt() {        
        TestDBO a;
        a.fields();
        
        EzSql::Stmt stmt(db);
        QVERIFY(stmt.prepare("select * from Test where id = :id") == SQLITE_OK);
        QVERIFY(stmt.bind(":id", 1) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_ROW);

        auto result = stmt.result();
        a.set(result);

        QVERIFY(a._id == 1);
        QVERIFY(a._blob == QByteArray("test data"));
        QVERIFY(a._boolean == false);
        QVERIFY(a._double == 0.13);
        QVERIFY(a._int64 == 13);
        QVERIFY(a._text == QString("test value"));
    }
};

QTEST_MAIN(TestBaseDBO)
#include "test_basedbo.moc"