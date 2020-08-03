#include <stdlib.h>
#include <QToolTip>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

#include "unlockdialog.h"
#include "ui_unlockdialog.h"
#include "tokendb.h"

extern tokendb db;

extern "C" {
#include "totp.h"
#include <liboath/oath.h>
#include <sodium.h>
}

unlockdialog::unlockdialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::unlockdialog)
{
    ui->setupUi(this);
    setTokenList();
}

unlockdialog::~unlockdialog()
{
    delete ui;
}

void unlockdialog::setTokenList()
{
    ui->tokenSelectBox->clear();

    QStringList tokenlist;
    QSqlError err;

    db.list_tokens(tokenlist, &err);
    if(!(err.type() == QSqlError::NoError)) {
        QMessageBox::critical(nullptr, "Error", QString("A critical error occurred opening token database: %1").arg(err.text()));
        this->done(QDialog::Rejected);
        return;
    }

    if(tokenlist.count() > 1) ui->tokenSelectBox->setEnabled(true);
    ui->tokenSelectBox->addItems(tokenlist);
}

void unlockdialog::on_unlockButton_clicked()
{
    ui->unlockButton->setEnabled(false);
    ui->passwordField->setEnabled(false);

    int rc;
    char code[8];
    unsigned char *otp_secret = 0;
    unsigned char crypto_key[crypto_secretbox_KEYBYTES];
    QPoint passwordFieldLoc;
    QString password = ui->passwordField->text();
    QSqlError err;

    if(password.isEmpty()) {
        passwordFieldLoc = ui->passwordField->mapToGlobal(QPoint());
        ui->unlockButton->setEnabled(true);
        ui->passwordField->setEnabled(true);
        ui->passwordField->setFocus();
        QToolTip::showText(passwordFieldLoc, "You must enter a password.", this, QRect(), 5000);
        return;
    }

    tokendb::token_s *token = new tokendb::token_s;
    db.get_token(ui->tokenSelectBox->currentText(), token, &err);
    if(!(err.type() == QSqlError::NoError)) {
        delete token;
        QMessageBox::critical(this, "Database error", QString("Error retreiving token from database: %1").arg(err.text()));
        return;
    }
    rc = sodium_init();
    if(-1 == rc) {
        delete token;
        QMessageBox::critical(this, "Error", "Fatal error occurred: unable to initialize libsodium. Please restart the application.");
        this->done(QDialog::Rejected);
        return;
    }

    rc = crypto_pwhash(crypto_key, sizeof(crypto_key), password.toLocal8Bit().constData(), password.length(),
                       (unsigned char *)token->kdf_salt.data(), crypto_pwhash_OPSLIMIT_MODERATE,
                       crypto_pwhash_MEMLIMIT_MODERATE, crypto_pwhash_ALG_ARGON2I13);
    qDebug() << "pwhash: " << rc << "value: " << QByteArray((char *)crypto_key, sizeof(crypto_key)).toHex();
    if(rc) goto err;

    otp_secret = new unsigned char[token->secret_len];
    rc = crypto_secretbox_open_easy((unsigned char *)otp_secret, (unsigned char *)token->secret.data(), token->secret.length(),
                                    (unsigned char *)token->iv.data(), crypto_key);
    qDebug() << "open: " << rc;
    if(rc) goto err;

    else {
        rc = generate_totp((char *)otp_secret, token->secret_len, code, 6);
        delete token;
        delete [] otp_secret;
        if(rc != OATH_OK) {
            QMessageBox::critical(this, "TOTP Error", QString("An error ocurred generating your token: %1").arg(oath_strerror(rc)));
            ui->passwordField->setEnabled(true);
            ui->passwordField->setFocus();
            ui->passwordField->selectAll();
            ui->unlockButton->setEnabled(true);
            return;
        } else {
            codedialog codedlg;
            codedlg.setAttribute(Qt::WA_DeleteOnClose);
            connect(&codedlg, &codedialog::finished, this, &unlockdialog::accept);
            codedlg.setTokenVal(code);
            codedlg.show();
            this->hide();
            return;
            //this->done(QDialog::Accepted);
        }
    }

    err:
    qDebug() << "got to err";
    delete token;
    delete [] otp_secret;
    ui->passwordField->setEnabled(true);
    ui->passwordField->setFocus();
    ui->passwordField->selectAll();
    ui->unlockButton->setEnabled(true);
    passwordFieldLoc = ui->passwordField->mapToGlobal(QPoint());
    QToolTip::showText(passwordFieldLoc, "Invalid password. Please check your password and try again.", this, QRect(), 5000);
}

void unlockdialog::on_setupButton_clicked()
{
    setupdialog setup;
    connect(&setup, &setupdialog::accepted, this, &unlockdialog::setTokenList);
    setup.exec();
}
