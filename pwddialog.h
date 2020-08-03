#ifndef PWDDIALOG_H
#define PWDDIALOG_H

#include <QDialog>

namespace Ui {
class pwddialog;
}

class pwddialog : public QDialog
{
    Q_OBJECT

public:
    explicit pwddialog(QWidget *parent = nullptr);
    ~pwddialog();

signals:
    void passwordEntered(const QString &password);

private slots:
    void on_passwordField_textChanged(const QString &arg1);

    void on_saveButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::pwddialog *ui;
};

#endif // PWDDIALOG_H
