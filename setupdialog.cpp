#include <QMessageBox>
#include <QDate>
#include <QString>
#include <QDebug>

#include "setupdialog.h"
#include "ui_setupdialog.h"
#include "unlockdialog.h"
#include "tokendb.h"

extern tokendb db;

extern "C" {
#include "enc.h"
#include "totp.h"
#include <sodium.h>
#include <liboath/oath.h>
}

setupdialog::setupdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setupdialog)
{
    ui->setupUi(this);
}

setupdialog::~setupdialog()
{
    delete [] password;
    delete ui;
}

void setupdialog::setpassword(const QString &in)
{
    passwordlen = in.length();
    password = new char[passwordlen];
    password = in.toLocal8Bit().constData();
}

void setupdialog::on_createButton_clicked()
{
    ui->createButton->setEnabled(false);
    ui->redoButton->setEnabled(false);
    ui->secretField->setEnabled(false);
    ui->friendlyNameField->setEnabled(false);

    int rc;
    const char *secret;
    char code[6];

    if(ui->friendlyNameField->text().isEmpty())
        generate_token_name();

    secret = ui->secretField->text().toLocal8Bit();
    rc = generate_totp(secret, ui->secretField->text().length(), code, 6);
    if(rc != OATH_OK) {
        QMessageBox::critical(nullptr, "Error", QString("An error occurred generating the token: %1. Please try again.").arg(oath_strerror(rc)));
        ui->createButton->setEnabled(true);
        ui->redoButton->setEnabled(true);
        ui->secretField->setEnabled(true);
        ui->friendlyNameField->setEnabled(true);
    } else { /* Token generation OK */
        ui->testCodeField->setText(code);
        ui->testCodeField->selectAll();
        ui->saveButton->setEnabled(true);
        ui->redoButton->setEnabled(true);
    }
}

void setupdialog::on_secretField_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        ui->createButton->setEnabled(false);
    else
        ui->createButton->setEnabled(true);
}

void setupdialog::on_redoButton_clicked()
{
    ui->saveButton->setEnabled(false);
    ui->createButton->setEnabled(true);
    ui->friendlyNameField->setEnabled(true);
    ui->secretField->setEnabled(true);
    ui->secretField->setText("");
    ui->testCodeField->setText("");
}

void setupdialog::on_saveButton_clicked()
{
    pwddialog pwddlg;
    pwddlg.setAttribute(Qt::WA_DeleteOnClose);
    pwddlg.setModal(true);
    connect(&pwddlg, &pwddialog::passwordEntered, this, &setupdialog::setpassword);
    if(pwddlg.exec() == QDialog::Rejected)
        return;

    int rc;
    unsigned char salt[crypto_pwhash_SALTBYTES];
    unsigned char iv[crypto_secretbox_NONCEBYTES];
    unsigned char key[crypto_secretbox_KEYBYTES];
    int token_secret_len = ui->secretField->text().length();
    unsigned char ciphertext[crypto_secretbox_MACBYTES + token_secret_len];
    tokendb::token_s *newtoken = new tokendb::token_s;
    QSqlError err;

    rc = sodium_init();
    if(-1 == rc) {
        delete newtoken;
        QMessageBox::critical(this, "Error", "Fatal error occurred: unable to initialize libsodium. Please restart the application.");
        this->done(QDialog::Rejected);
        return;
    }

    randombytes_buf(salt, sizeof(salt));
    randombytes_buf(iv, sizeof(iv));

    rc = crypto_pwhash(key, sizeof(key), password, passwordlen, salt,
                  crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
                  crypto_pwhash_ALG_ARGON2I13);
    qDebug() << "pwhash: " << rc << "value: " << QByteArray((char *)key, sizeof(key)).toHex();
    if(rc) {
        delete newtoken;
        QMessageBox::critical(this, "Error", "Encryption error occurred!");
        ui->redoButton->click();
    }

    rc = crypto_secretbox_easy(ciphertext, (unsigned char *)ui->secretField->text().toLocal8Bit().data(),
                          token_secret_len, iv, key);
    if(rc) {
        delete newtoken;
        QMessageBox::critical(this, "Error", "Encryption error occurred!");
        ui->redoButton->click();
    }
    qDebug() << "secretbox: " << rc;


    newtoken->num_digits = 6;
    newtoken->kdf_salt = QByteArray((char *)salt, sizeof(salt));
    newtoken->iv = QByteArray((char *)iv, sizeof(iv));
    newtoken->secret = QByteArray((char *)ciphertext, sizeof(ciphertext));
    newtoken->secret_len = token_secret_len;

    QString secret_str, iv_str, kdf_salt_str;
    secret_str = QByteArray((char *)ciphertext, sizeof(ciphertext)).toHex();
    iv_str = QByteArray((char *)iv, sizeof(iv)).toHex();
    kdf_salt_str = QByteArray((char *)salt, sizeof(salt)).toHex();
    qDebug() << "secret " << secret_str << "iv " << iv_str << "kdf_salt " << kdf_salt_str;

    err = db.create_token(ui->friendlyNameField->text(), newtoken);
    delete newtoken;
    if(err.type() == QSqlError::NoError) {
        this->done(QDialog::Accepted);
        return;
    } else {
        QMessageBox::critical(this, "Error", QString("Database error: %1").arg(err.text()));
        ui->redoButton->click();
        return;
    } 
}

void setupdialog::generate_token_name()
{
    int i;
    QString date = QDate::currentDate().toString(Qt::ISODate);
    QString name = QString("My Token %1").arg(date);

    i = 1;
    while(db.token_exists(name)) {
        name = QString("My Token %1 (%2)").arg(date, QString::number(i++));
    }
    ui->friendlyNameField->setText(name);
}
