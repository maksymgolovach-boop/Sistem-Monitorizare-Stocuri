#ifndef TRANSACTIONDIALOG_H
#define TRANSACTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QDateTime>
#include <QUuid>
#include "../NivelStocareDate/warehousemanager.h" // Avem nevoie de lista de produse

struct TransactionData {
    QString id;
    QDateTime timestamp;
    TipTranzactie tip;
    QString produsId;
    int cantitate;
    double pretUnitar;
    QString companie;
};

class TransactionDialog : public QDialog {
    Q_OBJECT

public:
    // Transmițând referința la depozit, putem popula ComboBox-ul cu produse reale
    explicit TransactionDialog(const WarehouseManager& depozit, QWidget *parent = nullptr);
    TransactionData getTransactionData() const;

private:
    TipTranzactie m_tipCurent = TipTranzactie::Achizitionare;
    QString m_generatedId;
    QDateTime m_currentTimestamp;

    QPushButton *btnTabAchizitie;
    QPushButton *btnTabVanzare;
    QComboBox *comboProduse;
    QSpinBox *spinCantitate;
    QDoubleSpinBox *spinPret;
    QLineEdit *editCompanie;
    QPushButton *btnConfirm;

    void setupUI(const WarehouseManager& depozit);
    void valideazaDatele();
    void updateToggleStyle();
};

#endif // TRANSACTIONDIALOG_H
