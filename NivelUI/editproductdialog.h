#ifndef EDITPRODUCTDIALOG_H
#define EDITPRODUCTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>

#include "../LibrarieModele/Produs.h"

/**
 * @brief Dialog pentru modificarea unui produs existent.
 *
 * Primește un const Produs& pentru a pre-popula câmpurile.
 * ID-ul este read-only — nu poate fi modificat.
 * Returnează un Produs actualizat prin getProdusCuModificari().
 *
 * Utilizare în MainWindow:
 * @code
 *   const Produs *p = depozit.gasesteProdusDupaId(id);
 *   EditProductDialog dialog(*p, this);
 *   if (dialog.exec() == QDialog::Accepted) {
 *       depozit.actualizeazaProdus(dialog.getProdusCuModificari());
 *       depozit.salveazaDate();
 *       populateProductsTable();
 *   }
 * @endcode
 */
class EditProductDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditProductDialog(const Produs &produs, QWidget *parent = nullptr);

    /**
     * @brief Returnează un Produs nou cu datele modificate de utilizator.
     *        ID-ul este păstrat identic cu cel al produsului original.
     */
    Produs getProdusCuModificari() const;

private:
    Produs m_produsOriginal;   // copie a produsului primit — păstrăm ID-ul

    QLabel         *lblIdValue;
    QLineEdit      *editNume;
    QSpinBox       *spinCantitate;
    QDoubleSpinBox *spinPret;
    QSpinBox       *spinPrag;
    QPushButton    *btnSave;
    QPushButton    *btnCancel;

    void setupUI();
    void populateCampuri();    // pre-completează câmpurile cu valorile produsului
};

#endif // EDITPRODUCTDIALOG_H
