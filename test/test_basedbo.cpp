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

class TestBaseDBO : public QObject {
    Q_OBJECT

  private slots:
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
        qDebug() << a.createStmt();
    }
};

QTEST_MAIN(TestBaseDBO)
#include "test_basedbo.moc"