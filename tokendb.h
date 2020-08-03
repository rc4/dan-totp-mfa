#ifndef TOKENDB_H
#define TOKENDB_H

#include <QSqlError>
#include <QSqlDatabase>

class tokendb
{
public:
    struct token_s {
        int num_digits;
        QByteArray kdf_salt;
        QByteArray iv;
        QByteArray secret;
        int secret_len;
    };

    tokendb();
    ~tokendb();
    //static tokendb *instance();
    bool isOpen() const;
    bool connect(const char *path, QSqlError &err);
    QSqlError init_table();
    QSqlError create_token(const QString name, token_s *in);
    bool get_token(QString token_name, token_s *ret, QSqlError *err);
    bool token_exists(QString token_name);
    bool list_tokens(QStringList &ret, QSqlError *err);
private:
    QSqlDatabase db;
};

#endif // TOKENDB_H
