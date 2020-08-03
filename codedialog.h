#ifndef CODEDIALOG_H
#define CODEDIALOG_H

#include <QDialog>

namespace Ui {
class codedialog;
}

class codedialog : public QDialog
{
    Q_OBJECT

public:
    explicit codedialog(QWidget *parent = nullptr);
    ~codedialog();
    void setTokenVal(char *);

private slots:
    void on_copyButton_clicked();

    void on_closeButton_clicked();

private:
    Ui::codedialog *ui;
};

#endif // CODEDIALOG_H
