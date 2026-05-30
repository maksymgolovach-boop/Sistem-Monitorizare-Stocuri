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
#include <QMessageBox>

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
    TipTranzactie       m_tipCurent = TipTranzactie::Achizitionare;
    QString             m_generatedId;
    QDateTime           m_currentTimestamp;
    const WarehouseManager &m_depozit;   // necesar pentru verificarea stocului la Vânzare

    QPushButton *btnTabAchizitie;
    QPushButton *btnTabVanzare;
    QComboBox *comboProduse;
    QSpinBox *spinCantitate;
    QDoubleSpinBox *spinPret;
    QLineEdit *editCompanie;
    QPushButton *btnConfirm;

    void setupUI(const WarehouseManager& depozit);

    /**
     * @brief Validează toate câmpurile obligatorii ale formularului.
     *
     * Verifică, în ordine:
     *  1. Produs selectat (nu placeholder-ul gol)
     *  2. Cantitate >= 1
     *  3. Preț unitar > 0
     *  4. Companie parteneră ne-goală
     *  5. Stoc suficient (doar la Vânzare)
     *
     * La primul câmp invalid marchează câmpul cu border roșu,
     * afișează un QMessageBox::warning și returnează false.
     * Dacă totul este valid, resetează stilurile și returnează true.
     *
     * @return true  — toate câmpurile sunt valide
     * @return false — cel puțin un câmp este invalid
     */
    bool valideazaDatele();

    void updateToggleStyle();
};

#endif // TRANSACTIONDIALOG_H
