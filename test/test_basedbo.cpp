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

        id("id", &_id, "", "NOT NULL PRIMARY KEY AUTOINCREMENT");
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

     void create_stmt()
    {
        TestDBO a;

        EzSql::Stmt stmt(db);
        QVERIFY(stmt.prepare(a.createStmt()) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_DONE);
        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    void insert_stmt()
    {
        TestDBO a;
        a._int64 = 1233333;
        a._boolean = false;
        a._text = "something data";  
        a._double = 0.13;
        
        EzSql::Stmt stmt(db);
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
        a._int64 = 0;
        a._boolean = false;
        a._double = 0.0;

        EzSql::Stmt stmt(db);
        qDebug() << a.updateStmt();
        QVERIFY(stmt.prepare(a.updateStmt()) == SQLITE_OK);
        QVERIFY(a.bind(stmt, a.allFieldName()) == SQLITE_OK);
        QVERIFY(stmt.step() == SQLITE_DONE);
        QVERIFY(stmt.finalize() == SQLITE_OK);
    }

    // void update_stmt_bind_name()
    // {
    //     TestDBO a;

    //     EzSql::Stmt stmt(db);
    //     QVERIFY(stmt.prepare(a.updateStmt()) == SQLITE_OK);
    //     QVERIFY(a.bind(stmt) == SQLITE_OK);
    //     QVERIFY(stmt.step() == SQLITE_DONE);
    //     QVERIFY(stmt.finalize() == SQLITE_OK);
    // }
};

QTEST_MAIN(TestBaseDBO)
#include "test_basedbo.moc"
