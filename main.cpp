#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QDebug>

#include "unlockdialog.h"
#include "setupdialog.h"
#include "tokendb.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    int reply;
    QStringList tokenlist;
    QSqlError err;

    extern tokendb db;
    db.connect("/home/daniel/tokens.db", err);

    if(db.isOpen()) {
        db.init_table(); // create if it doesn't exist
        db.list_tokens(tokenlist, &err);
    } else {
        QMessageBox::critical(nullptr, "Error", QString("A critical error occurred opening token database: %1").arg(err.text()));
        QApplication::quit();
    }

    if(tokenlist.isEmpty()) {
        reply = QMessageBox::question(nullptr, "No tokens found",
                                          "No tokens were found. Would you like to set one up now?",
                                          QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes) goto setup;
        else QApplication::quit();
    } else
        goto ok;

    setup: {
        setupdialog s;
        if(s.exec() == QDialog::Rejected) {
            QApplication::quit();
            return 1;
        }
        s.close();
        unlockdialog u;
        u.show();
        return a.exec();
    }
    ok: {
        unlockdialog u;
        u.show();
        return a.exec();
    }
}

//void QApplication::aboutToQuit()
//{
//    delete u;
//    delete s;
//}
