#include "exportmanager.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTextDocument>
#include <QPrinter>

// ─────────────────────────────────────────────────────────────────────────────
// exportCSV
// ─────────────────────────────────────────────────────────────────────────────

bool ExportManager::exportCSV(const QString &caleFisier, QTableWidget *table)
{
    if (!table) return false;

    QFile file(caleFisier);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // BOM UTF-8 — Excel îl folosește ca indiciu pentru encoding
    out << "\xEF\xBB\xBF";

    // ── Header ────────────────────────────────────────────────────────────────
    QStringList headerCols;
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem *h = table->horizontalHeaderItem(col);
        headerCols << csvQuote(h ? h->text() : QString("Col%1").arg(col));
    }
    out << headerCols.join(',') << '\n';

    // ── Rânduri vizibile ──────────────────────────────────────────────────────
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->isRowHidden(row)) continue;

        QStringList rowCols;
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *item = table->item(row, col);
            rowCols << csvQuote(item ? item->text() : QString());
        }
        out << rowCols.join(',') << '\n';
    }

    file.close();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// exportPDF
// ─────────────────────────────────────────────────────────────────────────────

bool ExportManager::exportPDF(const QString     &caleFisier,
                               QTableWidget      *table,
                               const QString     &titlu,
                               const QStringList &infoLines)
{
    if (!table) return false;
    return exportHTMLtoPDF(caleFisier,
                           buildHTML(table, titlu, infoLines),
                           /*landscape=*/true);
}

bool ExportManager::exportHTMLtoPDF(const QString &caleFisier,
                                     const QString &html,
                                     bool           landscape)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(caleFisier);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(landscape ? QPageLayout::Landscape
                                         : QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    return QFile::exists(caleFisier);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildHTML  (privat)
// ─────────────────────────────────────────────────────────────────────────────

QString ExportManager::buildHTML(QTableWidget      *table,
                                  const QString     &titlu,
                                  const QStringList &infoLines)
{
    const QString now = QDateTime::currentDateTime()
                            .toString("dd/MM/yyyy HH:mm");

    // ── CSS ───────────────────────────────────────────────────────────────────
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: Arial, Helvetica, sans-serif;
    font-size: 9pt;
    color: #212529;
  }
  .report-header {
    border-bottom: 2px solid #343a40;
    padding-bottom: 8px;
    margin-bottom: 10px;
  }
  .report-header h1 {
    font-size: 15pt;
    color: #212529;
    margin-bottom: 3px;
  }
  .report-header .meta {
    font-size: 8.5pt;
    color: #6c757d;
  }
  .info-block {
    margin-bottom: 10px;
  }
  .info-line {
    font-size: 9pt;
    color: #495057;
    margin: 2px 0;
  }
  table {
    border-collapse: collapse;
    width: 100%;
  }
  thead tr {
    background-color: #343a40;
    color: white;
  }
  th {
    padding: 6px 8px;
    text-align: left;
    font-size: 8.5pt;
    font-weight: bold;
  }
  td {
    padding: 5px 8px;
    border-bottom: 1px solid #dee2e6;
    font-size: 8pt;
  }
  tr.even-row td {
    background-color: #f8f9fa;
  }
  .report-footer {
    margin-top: 12px;
    font-size: 7.5pt;
    color: #adb5bd;
    border-top: 1px solid #dee2e6;
    padding-top: 5px;
  }
</style>
</head>
<body>
)";

    // ── Header raport ─────────────────────────────────────────────────────────
    html += "<div class=\"report-header\">";
    html += "<h1>" + titlu.toHtmlEscaped() + "</h1>";
    html += "<div class=\"meta\">Generat la: " + now + "</div>";
    html += "</div>\n";

    // ── Linii de info ─────────────────────────────────────────────────────────
    if (!infoLines.isEmpty()) {
        html += "<div class=\"info-block\">";
        for (const QString &line : infoLines)
            html += "<div class=\"info-line\">" + line.toHtmlEscaped() + "</div>";
        html += "</div>\n";
    }

    // ── Tabel ─────────────────────────────────────────────────────────────────
    html += "<table>\n<thead>\n<tr>";
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem *h = table->horizontalHeaderItem(col);
        html += "<th>" + (h ? h->text().toHtmlEscaped() : "") + "</th>";
    }
    html += "</tr>\n</thead>\n<tbody>\n";

    int visibleRow = 0;
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->isRowHidden(row)) continue;

        const QString rowClass = (visibleRow % 2 == 1) ? " class=\"even-row\"" : "";
        html += "<tr" + rowClass + ">";

        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *item = table->item(row, col);
            QString text = item ? item->text().toHtmlEscaped() : "";

            // Preia culoarea de prim-plan setată explicit pe celulă
            QString style;
            if (item) {
                QColor fg = item->foreground().color();
                if (fg.isValid())
                    style = QString(" style=\"color:%1;font-weight:bold\"")
                                .arg(fg.name());
            }

            html += "<td" + style + ">" + text + "</td>";
        }
        html += "</tr>\n";
        ++visibleRow;
    }

    html += "</tbody>\n</table>\n";

    // ── Footer ────────────────────────────────────────────────────────────────
    html += "<div class=\"report-footer\">";
    html += QString("Sistem Monitorizare Stocuri &mdash; raport automat &mdash; %1 "
                    "(%2 înregistrări exportate)")
                .arg(now)
                .arg(visibleRow);
    html += "</div>\n</body>\n</html>";

    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// csvQuote  (privat)
// ─────────────────────────────────────────────────────────────────────────────

QString ExportManager::csvQuote(const QString &s)
{
    // Înconjură cu ghilimele duble și escapează ghilimelele interne prin dublare
    return '"' + QString(s).replace('"', QLatin1String("\"\"")) + '"';
}
