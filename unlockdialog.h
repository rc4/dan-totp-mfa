#ifndef UNLOCKDIALOG_H
#define UNLOCKDIALOG_H

#include <QDialog>

#include "setupdialog.h"
#include "codedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class unlockdialog; }
QT_END_NAMESPACE

class unlockdialog : public QDialog
{
    Q_OBJECT

public:
    unlockdialog(QWidget *parent = nullptr);
    ~unlockdialog();
    void setTokenList();

private slots:
    void on_unlockButton_clicked();

    void on_setupButton_clicked();

private:
    Ui::unlockdialog *ui;
    setupdialog *setup;
    codedialog *codedlg;
};
#endif // UNLOCKDIALOG_H
