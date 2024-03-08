#ifndef EZSQL_H
#define EZSQL_H

#include <sqlite3.h>

#include <QDebug>
#include <QMap>
#include <QString>

namespace EzSql {

class Result {
  public:
    explicit Result(sqlite3_stmt* stmt = nullptr);

    bool map(QString& value, int idx = 0);
    bool map(QByteArray& value, int idx = 0);
    bool map(qint32& value, int idx = 0);
    bool map(qint64& value, int idx = 0);
    bool map(double& value, int idx = 0);
    bool map(bool& value, int idx = 0);
    qint32 count() const { return _count; }

    template <typename T>
    bool map(T& value, const QString& name)
    {
        if (!_stmt) {
            return false;
        }

        if (!_columns.contains(name)) {
            return false;
        }

        return map(value, _columns[name]);
    }

  private:
    QHash<QString, qint32> _columns;  // <colname, index>
    sqlite3_stmt* _stmt = nullptr;
    qint32 _count = 0;
};

class Stmt {
  public:
    explicit Stmt(sqlite3* db = nullptr);
    int prepare(const QString& query);

    int bind(int idx, const QByteArray& value);
    int bind(int idx, const QString& value);
    int bind(int idx, qint32 value);
    int bind(int idx, qint64 value);
    int bind(int idx, double value);

    template <typename T>
    int bind(const QString& name, const T& value)
    {
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

    inline void db(sqlite3* db) { _db = db; }

  private:
    sqlite3_stmt* _stmt = nullptr;
    sqlite3* _db = nullptr;
};

class BaseField {
  public:
    BaseField() = default;
    virtual ~BaseField() = default;

    QString name() const { return _name; }
    QString sqlType() const { return _sqlType; }
    QString columnConstraint() const { return _columnConstraint; }

    virtual void set(Result& result, const QString& prefixColName = "") = 0;
    virtual int bind(Stmt& stmt, qint32 idx) = 0;
    virtual int bind(Stmt& stmt) = 0;

    QString sqlTypeFrom(const QString&) { return "TEXT"; }
    QString sqlTypeFrom(const QByteArray&) { return "BLOB"; }
    QString sqlTypeFrom(const qint32&) { return "INTEGER"; }
    QString sqlTypeFrom(const qint64&) { return "INTEGER"; }
    QString sqlTypeFrom(const double&) { return "REAL"; }
    QString sqlTypeFrom(const bool&) { return "INTEGER"; }

  protected:
    QString _name;
    QString _sqlType;
    QString _columnConstraint;
};

template <typename T>
class Field : public BaseField {
  public:
    Field(T* value, const QString& name, const QString& sqlType, const QString& constraint)
    {
        _value = value;
        _name = name;
        _sqlType = sqlType == "" ? sqlTypeFrom(*value) : sqlType;
        _columnConstraint = constraint;
    }

    void set(const T& value) { *_value = value; }
    T get() const { return *_value; }

    void set(Result& result, const QString& prefixColName = "") override
    {
        result.map(*_value, prefixColName + _name);
    }

    int bind(Stmt& stmt, qint32 idx) override { return stmt.bind(idx, *_value); }
    int bind(Stmt& stmt) override { return stmt.bind(":" + _name, *_value); }

  private:
    T* _value;
};

class BaseDBO {
  public:
    BaseDBO() = default;
    virtual ~BaseDBO() = default;

    template <typename T>
    void set(const QString& name, const T& value)
    {
        if (_id && name == _id->name()) {
            if (Field<T>* field = dynamic_cast<Field<T>*>(_id)) {
                field->set(value);
            }
            else {
                Q_ASSERT_X(false, "BaseDBO::set<T>", "invalid cast Field<T>* id");
            }

            return;
        }

        if (_field.contains(name)) {
            if (Field<T>* field = dynamic_cast<Field<T>*>(_field[name])) {
                field->set(value);
            }
            else {
                Q_ASSERT_X(false, "BaseDBO::set<T>", "invalid cast Field<T>*");
            }
        }
    }

    void set(Result& result)
    {
        if (_id) {
            _id->set(result);
        }

        QHashIterator<QString, BaseField*> it(_field);
        while (it.hasNext()) {
            it.next();
            it.value()->set(result);
        }
    }

    void set(const QString& name, Result& result, const QString& prefixName = "")
    {
        if (_id && _id->name() == name) {
            _id->set(result);
            return;
        }

        if (_field.contains(name)) {
            _field[name]->set(result, prefixName);
        }
    }

    void setId(Result& result)
    {
        if (_id) {
            _id->set(result);
        }
    }

    int bind(Stmt& stmt, qint32 baseIdx = 0)
    {
        int rs;
        qint32 numValues = _field.size();
        qint32 idx = baseIdx;
        if (_id && !_id->columnConstraint().toUpper().contains("AUTOINCREMENT")) {
            idx += 1;
            rs = _id->bind(stmt, idx);
            if(rs != SQLITE_OK) {
                return rs;
            }
        }

        QHashIterator<QString, BaseField*> it(_field);
        while (it.hasNext()) {
            it.next();
            idx += 1;
            rs = it.value()->bind(stmt, idx);
            if(rs != SQLITE_OK) {
                return rs;
            }
        }

        return SQLITE_OK;
    }

    int bind(Stmt &stmt, const QStringList &fields)
    {
        int rs;

        for (auto &col : fields) {
            if (_id->name() == col) {
                rs = _id->bind(stmt);
                if (rs != SQLITE_OK) {
                    return rs;
                }
                continue;
            }
            if (_field.contains(col)) {
                rs = _field[col]->bind(stmt);
                if (rs != SQLITE_OK) {
                    return rs;
                }
            }
        }

        return SQLITE_OK;
    }

    template <typename T>
    T get(const QString& name) const
    {
        T rs;
        if (_id && name == _id->name()) {
            if (Field<T>* field = dynamic_cast<Field<T>*>(_id)) {
                rs = field->get();
            }
            else {
                Q_ASSERT_X(false, "BaseDBO::set<T>", "invalid cast Field<T>* id");
            }

            return rs;
        }

        if (_field.contains(name)) {
            if (Field<T>* field = dynamic_cast<Field<T>*>(_field[name])) {
                rs = field->get();
            }
            else {
                Q_ASSERT_X(false, "BaseDBO::get<T>", "invalid cast Field<T>*");
            }
        }

        return rs;
    }

    QString createStmt()
    {
        fields();
        QString listFields = "(";
        if (_id) {
            listFields = listFields + _id->name() + " " + _id->sqlType() + " " +
                         _id->columnConstraint() + ", ";
        }

        QHashIterator<QString, BaseField*> i(_field);
        while (i.hasNext()) {
            i.next();
            listFields = listFields + (i.key() + " " + i.value()->sqlType() + " " +
                                       i.value()->columnConstraint() + ", ");
        }
        listFields.chop(2);
        listFields += ")";

        QString stmt = "CREATE TABLE IF NOT EXISTS " + _tableName + " " + listFields;

        return stmt;
    }

    QString insertStmt(int numRecord = 1)
    {
        fields();

        int numValues = _field.size();
        QString returnning = "";
        QString listFields = "(";
        if (_id) {
            returnning = " RETURNING " + _id->name();

            if (!_id->columnConstraint().toUpper().contains("AUTOINCREMENT")) {
                listFields = listFields + _id->name() + ", ";
                numValues += 1;
            }
        }

        QHashIterator<QString, BaseField*> i(_field);
        while (i.hasNext()) {
            i.next();
            listFields = listFields + i.key() + ", ";
        }
        listFields.chop(2);
        listFields += ")";

        QString values = "(";
        for (int i = 0; i < numValues; i++) {
            values = values + "?, ";
        }
        values.chop(2);
        values += ")";

        QString records;
        for (int i = 0; i < numRecord; i++) {
            records = records + values + ", ";
        }
        records.chop(2);

        QString stmt = "INSERT INTO " + _tableName + listFields + " VALUES " + records + returnning;

        return stmt;
    }

    QString updateStmt(const QStringList& columns = {})
    {
        fields();

        int numValues = _field.size();
        QString listFields = "";
        if (!_id) {
            Q_ASSERT_X(false, "BaseDBO::updateStmt", "cant update dbo without id field");
        }

        if(!columns.isEmpty()) {
            for(const auto& iCol : columns) {
                if(_field.contains(iCol)) {
                    listFields = listFields + QString("%1=:%1, ").arg(iCol);
                }
            }
        }
        else {
            QHashIterator<QString, BaseField*> i(_field);
            while (i.hasNext()) {
                i.next();
                listFields = listFields + QString("%1=:%1, ").arg(i.key());
            }            
        }
        listFields.chop(2);
        
        QString stmt = "UPDATE " + _tableName + " SET " + listFields + " WHERE " + QString("%1=:%1").arg(_id->name());

        return stmt;
    }

    QStringList allFieldName() {
        QStringList fieldsName = _field.keys();
        fieldsName << _id->name();
        return fieldsName;
    }

  protected:
    template <typename T>
    void field(const QString& name, T* value, const QString& sqlType = "",
               const QString& constraint = "")
    {
        if (!_field.contains(name)) {
            BaseField* field = new Field(value, name, sqlType, constraint);
            _field.insert(name, field);
        }
    }

    template <typename T>
    void id(const QString& name, T* value, const QString& sqlType = "",
            const QString& constraint = "")
    {
        if (!_id) {
            BaseField* field = new Field(value, name, sqlType, constraint);
            _id = field;
        }
    }

    void table(const QString& name) { _tableName = name; }

    virtual void fields() = 0;

  private:
    QHash<QString, BaseField*> _field;
    BaseField* _id = nullptr;
    QString _tableName;
};

}  // namespace EzSql

#endif  // EZSQL_H
