#include <QClipboard>
#include <QDebug>
#include <QThread>

#include "codedialog.h"
#include "ui_codedialog.h"

codedialog::codedialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::codedialog)
{
    ui->setupUi(this);
}

void codedialog::setTokenVal(char *value)
{
    ui->tokenDisplayBox->setText(value);
    ui->tokenDisplayBox->selectAll();
}

codedialog::~codedialog()
{
    delete ui;
}

void codedialog::on_copyButton_clicked()
{
    QString code = ui->tokenDisplayBox->text();
    QClipboard *clipboard = QApplication::clipboard();
    qDebug() << "original cb text: " << clipboard->text() << " text to be copied: " << code;
    clipboard->clear(QClipboard::Clipboard);
    clipboard->setText(code, QClipboard::Clipboard);

    this->done(QDialog::Accepted);
    return;
}

void codedialog::on_closeButton_clicked()
{
    this->done(QDialog::Accepted);
    return;
}
