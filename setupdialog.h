#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>

#include "pwddialog.h"

namespace Ui {
class setupdialog;
}

class setupdialog : public QDialog
{
    Q_OBJECT

public:
    explicit setupdialog(QWidget *parent = nullptr);
    void setpassword(const QString &);
    ~setupdialog();

private slots:
    void on_createButton_clicked();

    void on_secretField_textChanged(const QString &arg1);

    void on_redoButton_clicked();

    void on_saveButton_clicked();

    void generate_token_name();
private:
    Ui::setupdialog *ui;
    pwddialog *pwddlg;
    const char *password;
    int passwordlen;
};

#endif // SETUPDIALOG_H
