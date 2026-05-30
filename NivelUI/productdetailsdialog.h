#ifndef PRODUCTDETAILSDIALOG_H
#define PRODUCTDETAILSDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <vector>
#include "../LibrarieModele/Produs.h"
#include "../LibrarieModele/Tranzactie.h"

/**
 * @brief Dialog read-only care afișează detaliile complete ale unui produs.
 *
 * Poate fi deschis cu double-click din orice tabel (Produse, Alerte, Dashboard).
 *
 * Secțiuni:
 *  - Header: nume, ID, badge status (OK / Sub Prag)
 *  - Carduri: Preț, Stoc Curent, Prag Alertă
 *  - Statistici: nr. tranzacții, valoare achiziții, valoare vânzări
 *  - Tabel: ultimele 10 tranzacții ale produsului (cele mai recente primele)
 */
class ProductDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param produs      Produsul ale cărui detalii vor fi afișate.
     * @param tranzactii  Toate tranzacțiile filtrate pentru acest produs.
     * @param parent      Widget-ul părinte.
     */
    explicit ProductDetailsDialog(const Produs                      &produs,
                                  const std::vector<TranzactieProdus> &tranzactii,
                                  QWidget                            *parent = nullptr);

private:
    void setupUI(const Produs &produs, const std::vector<TranzactieProdus> &tranzactii);

    /** Creează un mini-card cu etichetă și valoare (folosit pentru Preț / Stoc / Prag). */
    QWidget *createInfoCard(const QString &label,
                            const QString &value,
                            const QString &valueColor = "#212529") const;
};

#endif // PRODUCTDETAILSDIALOG_H
