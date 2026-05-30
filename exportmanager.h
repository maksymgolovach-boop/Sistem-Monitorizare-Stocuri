#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <QString>
#include <QStringList>
#include <QTableWidget>

/**
 * @brief Utilitar static pentru exportul oricărui QTableWidget în CSV sau PDF.
 *
 * Ambele metode lucrează cu rândurile vizibile (neascunse) ale tabelului,
 * deci respectă automat filtrul de căutare activ.
 *
 * Culorile de prim-plan definite explicit pe celule (QTableWidgetItem::setForeground)
 * sunt preluate în PDF ca inline-style CSS — nu este nevoie de configurare suplimentară.
 *
 * Utilizare:
 * @code
 *   // CSV
 *   ExportManager::exportCSV("alerte.csv", alertsTable);
 *
 *   // PDF cu linii de sumar deasupra tabelului
 *   ExportManager::exportPDF("raport.pdf", historyTable,
 *                            "Istoric Tranzacții",
 *                            {"Total: 42 tranzacții", "Val. vânzări: 1 200 RON"});
 * @endcode
 */
class ExportManager
{
public:
    /**
     * @brief Exportă rândurile vizibile ale tabelului într-un fișier CSV.
     *
     * Fișierul este scris cu BOM UTF-8 (compatibil Excel).
     * Câmpurile text sunt înconjurate de ghilimele duble;
     * ghilimelele interne sunt escapate prin dublare ("").
     *
     * @return true dacă fișierul a fost scris cu succes.
     */
    static bool exportCSV(const QString &caleFisier, QTableWidget *table);

    /**
     * @brief Generează un raport PDF din rândurile vizibile ale tabelului.
     *
     * @param titlu      Titlul raportului (apare ca heading în PDF).
     * @param infoLines  Linii de informații suplimentare (sumar, statistici etc.)
     *                   afișate între titlu și tabel. Poate fi gol.
     * @return true dacă fișierul PDF a fost generat cu succes.
     */
    static bool exportPDF(const QString    &caleFisier,
                          QTableWidget     *table,
                          const QString    &titlu,
                          const QStringList &infoLines = {});

    /**
     * @brief Randează un document HTML gata construit ca fișier PDF.
     *
     * Folosit intern de exportPDF() și de generateInvoice() din MainWindow.
     * @param landscape  true = orientare Landscape; false (implicit) = Portrait.
     */
    static bool exportHTMLtoPDF(const QString &caleFisier,
                                 const QString &html,
                                 bool           landscape = false);

private:
    /** Înconjură un câmp cu ghilimele duble și escapează ghilimelele interne. */
    static QString csvQuote(const QString &s);

    /** Construiește documentul HTML complet folosit la randarea PDF-ului. */
    static QString buildHTML(QTableWidget      *table,
                             const QString     &titlu,
                             const QStringList &infoLines);
};

#endif // EXPORTMANAGER_H
