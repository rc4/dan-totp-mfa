#include <QString>
#include <QDialog>
#include <QMessageBox>

#include "pwddialog.h"
#include "ui_pwddialog.h"

#define MIN_PASSWORD_LENGTH 10

pwddialog::pwddialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::pwddialog)
{
    ui->setupUi(this);
}

pwddialog::~pwddialog()
{
    delete ui;
}

void pwddialog::on_passwordField_textChanged(const QString &arg1)
{
    if(arg1.isEmpty()) {
        ui->confirmField->setEnabled(false);
        ui->saveButton->setEnabled(false);
    }
    else {
        ui->confirmField->setEnabled(true);
        ui->saveButton->setEnabled(true);
    }
}

void pwddialog::on_saveButton_clicked()
{
    ui->confirmField->setEnabled(false);
    ui->saveButton->setEnabled(false);

    QString password = ui->passwordField->text();
    QString confirm = ui->confirmField->text();

    if(password != confirm) {
        QMessageBox::critical(this, "Error", "Passwords do not match. Please try again.");
        ui->confirmField->setEnabled(true);
        ui->saveButton->setEnabled(true);
        return;
    } else if(password.length() < MIN_PASSWORD_LENGTH) {
        QMessageBox::critical(this, "Error", "Password entered was too short. Please try again.");
        ui->confirmField->setEnabled(true);
        ui->saveButton->setEnabled(true);
        return;
    } else {
        emit passwordEntered(password);
        this->done(QDialog::Accepted);
        return;
    }

}

void pwddialog::on_cancelButton_clicked()
{
    this->done(QDialog::Rejected);
    return;
}
