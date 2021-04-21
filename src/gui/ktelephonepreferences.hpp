#ifndef KTELEPHONEPREFERENCES_H
#define KTELEPHONEPREFERENCES_H

#include <QDialog>
#include <QListWidgetItem>

#include "ktelephone.hpp"

class KTelephoneManager;

namespace Ui {
    class settings;
}

class KTelephonePreferences : public QMainWindow {
Q_OBJECT

public:
    explicit KTelephonePreferences(QWidget *parent = 0);

    ~KTelephonePreferences();

    void show();
    void reload();
    void setManager(KTelephoneManager *manager);

private:
    Ui::settings *ui;

protected:
    Telephone_t mTelephone;
    KTelephoneManager *mManager;
    QListWidgetItem *currentTelephone;

public slots:

    void itemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void saveChanges();

    void newItem();

    void removeItem();
};

#endif // KTELEPHONEPREFERENCES_H
