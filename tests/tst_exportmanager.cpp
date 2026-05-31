#include <QtTest>
#include <QApplication>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include "exportmanager.h"

class TstExportManager : public QObject
{
    Q_OBJECT

private:
    // Helper: creeaza un QTableWidget cu coloane si date de test
    static QTableWidget *makeTable(QObject *parent = nullptr)
    {
        QTableWidget *table = new QTableWidget(3, 3, nullptr);

        // Headere orizontale
        table->setHorizontalHeaderItem(0, new QTableWidgetItem("Nume"));
        table->setHorizontalHeaderItem(1, new QTableWidgetItem("Cantitate"));
        table->setHorizontalHeaderItem(2, new QTableWidgetItem("Pret"));

        // Rand 0
        table->setItem(0, 0, new QTableWidgetItem("Lapte"));
        table->setItem(0, 1, new QTableWidgetItem("100"));
        table->setItem(0, 2, new QTableWidgetItem("3.50"));

        // Rand 1
        table->setItem(1, 0, new QTableWidgetItem("Paine"));
        table->setItem(1, 1, new QTableWidgetItem("50"));
        table->setItem(1, 2, new QTableWidgetItem("2.00"));

        // Rand 2
        table->setItem(2, 0, new QTableWidgetItem("Branza"));
        table->setItem(2, 1, new QTableWidgetItem("30"));
        table->setItem(2, 2, new QTableWidgetItem("12.50"));

        return table;
    }

    // Helper: citeste tot continutul fisierului ca QString
    static QString readFile(const QString &path)
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();
        QTextStream in(&f);
        in.setEncoding(QStringConverter::Utf8);
        return in.readAll();
    }

private slots:

    // ── exportCSV ─────────────────────────────────────────────────────────────

    void exportCSVCreatesFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/test.csv";

        QTableWidget *table = makeTable();
        bool ok = ExportManager::exportCSV(cale, table);
        delete table;

        QVERIFY(ok);
        QVERIFY(QFile::exists(cale));
    }

    void exportCSVHeaderRowMatchesHorizontalHeaders()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/headers.csv";

        QTableWidget *table = makeTable();
        ExportManager::exportCSV(cale, table);
        delete table;

        QString content = readFile(cale);
        // Primul rand (dupa BOM) trebuie sa contina headerele
        QStringList lines = content.split('\n', Qt::SkipEmptyParts);
        QVERIFY(lines.size() >= 1);
        QString headerLine = lines[0];
        QVERIFY(headerLine.contains("Nume"));
        QVERIFY(headerLine.contains("Cantitate"));
        QVERIFY(headerLine.contains("Pret"));
    }

    void exportCSVDataRowsMatchTableContent()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/data.csv";

        QTableWidget *table = makeTable();
        ExportManager::exportCSV(cale, table);
        delete table;

        QString content = readFile(cale);
        QStringList lines = content.split('\n', Qt::SkipEmptyParts);

        // 1 linie header + 3 linii date = 4 linii total
        QCOMPARE(lines.size(), 4);

        // Verifica ca datele sunt prezente
        QVERIFY(content.contains("Lapte"));
        QVERIFY(content.contains("100"));
        QVERIFY(content.contains("3.50"));
        QVERIFY(content.contains("Paine"));
        QVERIFY(content.contains("Branza"));
    }

    void exportCSVSkipsHiddenRows()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/hidden.csv";

        QTableWidget *table = makeTable();
        table->setRowHidden(1, true);  // ascunde randul cu "Paine"

        ExportManager::exportCSV(cale, table);
        delete table;

        QString content = readFile(cale);
        QStringList lines = content.split('\n', Qt::SkipEmptyParts);

        // 1 header + 2 randuri vizibile (rand 0 si rand 2)
        QCOMPARE(lines.size(), 3);
        QVERIFY(!content.contains("Paine"));
        QVERIFY(content.contains("Lapte"));
        QVERIFY(content.contains("Branza"));
    }

    void exportCSVQuotesFieldsContainingCommas()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/commas.csv";

        QTableWidget *table = new QTableWidget(1, 2);
        table->setHorizontalHeaderItem(0, new QTableWidgetItem("Coloana1"));
        table->setHorizontalHeaderItem(1, new QTableWidgetItem("Coloana2"));
        table->setItem(0, 0, new QTableWidgetItem("Valoare cu, virgula"));
        table->setItem(0, 1, new QTableWidgetItem("Normal"));

        ExportManager::exportCSV(cale, table);
        delete table;

        QString content = readFile(cale);
        // Campul cu virgula trebuie sa fie intre ghilimele
        QVERIFY(content.contains("\"Valoare cu, virgula\""));
    }

    // ── exportHTMLtoPDF ───────────────────────────────────────────────────────

    void exportHTMLtoPDFCreatesNonEmptyFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/test_html.pdf";

        QString html = "<html><body><h1>Test PDF</h1><p>Continut test.</p></body></html>";
        bool ok = ExportManager::exportHTMLtoPDF(cale, html, false);

        QVERIFY(ok);
        QVERIFY(QFile::exists(cale));
        QVERIFY(QFileInfo(cale).size() > 0);
    }

    void exportHTMLtoPDFLandscapeCreatesFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/landscape.pdf";

        QString html = "<html><body><table><tr><td>A</td><td>B</td></tr></table></body></html>";
        bool ok = ExportManager::exportHTMLtoPDF(cale, html, /*landscape=*/true);

        QVERIFY(ok);
        QVERIFY(QFile::exists(cale));
        QVERIFY(QFileInfo(cale).size() > 0);
    }

    // ── exportPDF ─────────────────────────────────────────────────────────────

    void exportPDFCreatesNonEmptyFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/raport.pdf";

        QTableWidget *table = makeTable();
        bool ok = ExportManager::exportPDF(cale, table,
                                           "Raport Test",
                                           {"Linie info 1", "Linie info 2"});
        delete table;

        QVERIFY(ok);
        QVERIFY(QFile::exists(cale));
        QVERIFY(QFileInfo(cale).size() > 0);
    }

    void exportPDFWithEmptyInfoLinesCreatesFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/raport_noinfo.pdf";

        QTableWidget *table = makeTable();
        bool ok = ExportManager::exportPDF(cale, table, "Raport Fara Info");
        delete table;

        QVERIFY(ok);
        QVERIFY(QFile::exists(cale));
        QVERIFY(QFileInfo(cale).size() > 0);
    }

    void exportPDFWithNullTableReturnsFalse()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/null.pdf";

        bool ok = ExportManager::exportPDF(cale, nullptr, "Test");
        QVERIFY(!ok);
    }

    void exportCSVWithNullTableReturnsFalse()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = dir.path() + "/null.csv";

        bool ok = ExportManager::exportCSV(cale, nullptr);
        QVERIFY(!ok);
    }
};

QTEST_MAIN(TstExportManager)
#include "tst_exportmanager.moc"
