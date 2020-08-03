#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include "tokendb.h"

tokendb db;

tokendb::tokendb()
{

}

tokendb::~tokendb()
{
    if(db.isOpen())
        db.close();
}

//tokendb *tokendb::instance()
//{
//    static tokendb instance;
//    return &instance;
//}

bool tokendb::connect(const char *path, QSqlError &err)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if(!db.open()) {
        return false;
        err = db.lastError();
        qDebug() << "Open error: " << db.lastError().text();
    }
    else
        return true;
}


bool tokendb::isOpen() const
{
    return db.isOpen();
}

QSqlError tokendb::init_table()
{
    QSqlQuery create;

    create.exec("CREATE TABLE IF NOT EXISTS tokens (" \
                   "version	INTEGER NOT NULL DEFAULT 1," \
                   "created	TIMESTAMP DEFAULT CURRENT_TIMESTAMP," \
                   "token_name	TEXT NOT NULL UNIQUE," \
                   "token_digits	INTEGER DEFAULT 6," \
                   "kdf_salt	BLOB NOT NULL," \
                   "iv	BLOB NOT NULL," \
                   "secret	BLOB NOT NULL," \
                   "secret_len INTEGER NOT NULL);");
    qDebug() << "Create: " << create.lastError().text();
    return create.lastError();
}

QSqlError tokendb::create_token(const QString name, token_s *in)
{
    qDebug() << "Got to start of create function";
    QSqlQuery insert;
    insert.prepare("INSERT INTO tokens" \
                   "(token_name, kdf_salt, iv, secret, secret_len)" \
                   "VALUES (:name, :kdf_salt, :iv, :secret, :secret_len);");
    insert.bindValue(":name", name);
    insert.bindValue(":kdf_salt", in->kdf_salt);
    insert.bindValue(":iv", in->iv);
    insert.bindValue(":secret", in->secret);
    insert.bindValue(":secret_len", in->secret_len);
    insert.exec();
    return insert.lastError();
}

bool tokendb::get_token(QString token_name, token_s *ret, QSqlError *err)
{
    QSqlQuery find;

    find.prepare("SELECT token_digits, kdf_salt, iv, secret, secret_len FROM tokens WHERE token_name = :name");
    find.bindValue(":name", token_name);

    if(!find.exec()) {
        *err = find.lastError();
        return false;
    }
    if(find.first()) {
        ret->num_digits = find.value(0).toInt();
        ret->kdf_salt = find.value(1).toByteArray();
        ret->iv = find.value(2).toByteArray();
        ret->secret = find.value(3).toByteArray();
        ret->secret_len = find.value(4).toInt();
        return true;
    }
    else return false; /* not found */
}

bool tokendb::token_exists(QString token_name)
{
    QSqlQuery find;

    find.prepare("SELECT * FROM tokens WHERE token_name = :name");
    find.bindValue(":name", token_name);

    if(!find.exec()) {
        qDebug() << "error checking if token exists: " << db.lastError().text();
        return true; /* error? */
    }
    return find.first();
}

bool tokendb::list_tokens(QStringList &ret, QSqlError *err)
{
    QSqlQuery list;

    list.prepare("SELECT token_name FROM tokens");

    if(!list.exec()) {
        qDebug() << "List err: " << list.lastError().text();
        *err = list.lastError();
        return false;
    }
    if(list.first()) {
        qDebug() << "List return: " << list.value(0);
        ret << list.value(0).toString();
        while(list.next())
            ret << list.value(0).toString();
        return true;
    }
    else return false; // No tokens in DB
}
