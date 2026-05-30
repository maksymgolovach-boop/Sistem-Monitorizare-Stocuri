#include "productdetailsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QHeaderView>
#include <QFont>
#include <QBrush>
#include <QColor>
#include <algorithm>

ProductDetailsDialog::ProductDetailsDialog(const Produs                       &produs,
                                           const std::vector<TranzactieProdus> &tranzactii,
                                           QWidget                             *parent)
    : QDialog(parent)
{
    setupUI(produs, tranzactii);
}

// ─────────────────────────────────────────────────────────────────────────────

void ProductDetailsDialog::setupUI(const Produs                       &produs,
                                   const std::vector<TranzactieProdus> &tranzactii)
{
    setFixedSize(660, 580);
    setWindowTitle("Detalii Produs");
    setStyleSheet("background-color: white;");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    // ── 1. HEADER ─────────────────────────────────────────────────────────────
    {
        QHBoxLayout *hdr = new QHBoxLayout();
        hdr->setSpacing(14);

        QLabel *icon = new QLabel("📦");
        icon->setStyleSheet(
            "font-size: 26px; background: #f8f9fa;"
            "padding: 10px; border-radius: 10px;");
        icon->setFixedSize(54, 54);
        icon->setAlignment(Qt::AlignCenter);

        QVBoxLayout *titleCol = new QVBoxLayout();
        titleCol->setSpacing(2);

        QLabel *lblNume = new QLabel(produs.nume());
        lblNume->setStyleSheet("font-size: 20px; font-weight: bold; color: #212529;");

        QLabel *lblId = new QLabel("ID: " + produs.id());
        lblId->setStyleSheet("font-size: 11px; color: #6c757d;");

        titleCol->addWidget(lblNume);
        titleCol->addWidget(lblId);

        // Badge status
        const bool subPrag = produs.esteSubPrag();
        QLabel *badge = new QLabel(subPrag ? "⚠  Sub Prag Alertă" : "✓  Stoc OK");
        badge->setStyleSheet(
            QString("background: %1; color: white; border-radius: 6px;"
                    "padding: 4px 10px; font-size: 11px; font-weight: bold;")
                .arg(subPrag ? "#dc3545" : "#198754"));
        badge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        hdr->addWidget(icon);
        hdr->addLayout(titleCol);
        hdr->addStretch();
        hdr->addWidget(badge);
        root->addLayout(hdr);
    }

    // Separator
    QFrame *sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #dee2e6;");
    root->addWidget(sep1);

    // ── 2. CARDURI INFO (Preț / Stoc / Prag) ─────────────────────────────────
    {
        QHBoxLayout *cards = new QHBoxLayout();
        cards->setSpacing(10);

        cards->addWidget(createInfoCard(
            "Categorie",
            produs.categorie(),
            "#0d6efd"));

        cards->addWidget(createInfoCard(
            "Preț Unitar",
            QString::number(produs.pret(), 'f', 2) + " RON"));

        const QString stocColor = produs.esteSubPrag() ? "#dc3545" : "#212529";
        cards->addWidget(createInfoCard(
            "Stoc Curent",
            QString::number(produs.cantitate()) + " buc.",
            stocColor));

        cards->addWidget(createInfoCard(
            "Prag Alertă",
            (produs.pragAlerta() > 0
                 ? QString::number(produs.pragAlerta()) + " buc."
                 : "Nesetat")));

        root->addLayout(cards);
    }

    // ── 3. STATISTICI TRANZACȚII ──────────────────────────────────────────────
    {
        int nrAchiz = 0, nrVanz = 0;
        double valAchiz = 0.0, valVanz = 0.0;
        for (const auto &t : tranzactii) {
            if (t.tip() == TipTranzactie::Achizitionare) {
                ++nrAchiz; valAchiz += t.valoareTotala();
            } else {
                ++nrVanz;  valVanz  += t.valoareTotala();
            }
        }

        QFrame *statBox = new QFrame();
        statBox->setStyleSheet(
            "background: #f8f9fa; border-radius: 8px;");

        QHBoxLayout *statLay = new QHBoxLayout(statBox);
        statLay->setContentsMargins(16, 10, 16, 10);
        statLay->setSpacing(0);

        auto addStat = [&](const QString &label, const QString &val) {
            QVBoxLayout *col = new QVBoxLayout();
            col->setSpacing(2);
            QLabel *lbl = new QLabel(label);
            lbl->setStyleSheet("color: #6c757d; font-size: 10px;");
            lbl->setAlignment(Qt::AlignCenter);
            QLabel *v = new QLabel(val);
            v->setStyleSheet("font-size: 14px; font-weight: bold; color: #212529;");
            v->setAlignment(Qt::AlignCenter);
            col->addWidget(lbl);
            col->addWidget(v);
            statLay->addLayout(col);
        };

        auto addDiv = [&]() {
            QFrame *d = new QFrame();
            d->setFrameShape(QFrame::VLine);
            d->setStyleSheet("color: #dee2e6;");
            statLay->addWidget(d);
        };

        addStat("Total tranzacții", QString::number(tranzactii.size()));
        addDiv();
        addStat("Achiziții",        QString::number(nrAchiz) + " op.");
        addDiv();
        addStat("Val. Achiziții",   QString::number(valAchiz, 'f', 2) + " RON");
        addDiv();
        addStat("Vânzări",          QString::number(nrVanz) + " op.");
        addDiv();
        addStat("Val. Vânzări",     QString::number(valVanz, 'f', 2) + " RON");

        root->addWidget(statBox);
    }

    // ── 4. TABEL TRANZACȚII (ultimele 10, cele mai recente primele) ───────────
    {
        QLabel *lblSec = new QLabel("Ultimele tranzacții");
        lblSec->setStyleSheet("font-size: 13px; font-weight: bold; color: #495057;");
        root->addWidget(lblSec);

        // Sortăm o copie după timestamp descrescător
        std::vector<TranzactieProdus> recente = tranzactii;
        std::sort(recente.begin(), recente.end(),
                  [](const TranzactieProdus &a, const TranzactieProdus &b) {
                      return a.timestamp() > b.timestamp();
                  });
        if (recente.size() > 10)
            recente.erase(recente.begin() + 10, recente.end());

        QTableWidget *tbl = new QTableWidget();
        tbl->setColumnCount(5);
        tbl->setHorizontalHeaderLabels({"Data/Ora", "Tip", "Companie", "Cantitate", "Total (RON)"});
        tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
        tbl->verticalHeader()->setVisible(false);
        tbl->setShowGrid(false);
        tbl->setFrameShape(QFrame::NoFrame);
        tbl->setObjectName("MainProductsTable");

        if (recente.empty()) {
            tbl->setRowCount(1);
            QTableWidgetItem *noData = new QTableWidgetItem("Nu există tranzacții înregistrate pentru acest produs.");
            noData->setTextAlignment(Qt::AlignCenter);
            noData->setForeground(QBrush(QColor("#6c757d")));
            tbl->setItem(0, 0, noData);
            tbl->setSpan(0, 0, 1, 5);
        } else {
            for (const auto &t : recente) {
                int row = tbl->rowCount();
                tbl->insertRow(row);

                tbl->setItem(row, 0, new QTableWidgetItem(
                    t.timestamp().toString("dd/MM/yyyy HH:mm")));

                QTableWidgetItem *tipItem = new QTableWidgetItem(
                    t.tip() == TipTranzactie::Achizitionare ? "➕ ACHIZIȚIE" : "➖ VÂNZARE");
                tipItem->setForeground(QBrush(QColor(
                    t.tip() == TipTranzactie::Achizitionare ? "#198754" : "#dc3545")));
                tipItem->setFont(QFont("Arial", 9, QFont::Bold));
                tbl->setItem(row, 1, tipItem);

                tbl->setItem(row, 2, new QTableWidgetItem(t.numeCompanie()));

                QTableWidgetItem *cantiItem = new QTableWidgetItem(
                    QString::number(t.cantitate()) + " buc.");
                cantiItem->setTextAlignment(Qt::AlignCenter);
                tbl->setItem(row, 3, cantiItem);

                QTableWidgetItem *totalItem = new QTableWidgetItem(
                    QString::number(t.valoareTotala(), 'f', 2));
                totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                tbl->setItem(row, 4, totalItem);
            }
        }

        root->addWidget(tbl);
    }

    // ── 5. FOOTER ─────────────────────────────────────────────────────────────
    {
        QHBoxLayout *footer = new QHBoxLayout();
        footer->addStretch();

        QPushButton *btnClose = new QPushButton("✕  Închide");
        btnClose->setObjectName("BtnDialogCancel");
        btnClose->setMinimumSize(110, 36);
        btnClose->setCursor(Qt::PointingHandCursor);
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

        footer->addWidget(btnClose);
        root->addLayout(footer);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

QWidget *ProductDetailsDialog::createInfoCard(const QString &label,
                                               const QString &value,
                                               const QString &valueColor) const
{
    QWidget *card = new QWidget();
    card->setStyleSheet(
        "background: #f8f9fa; border-radius: 8px;");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(14, 10, 14, 10);
    lay->setSpacing(4);

    QLabel *lbl = new QLabel(label);
    lbl->setStyleSheet("color: #6c757d; font-size: 11px;");

    QLabel *val = new QLabel(value);
    val->setStyleSheet(
        QString("color: %1; font-size: 18px; font-weight: bold;").arg(valueColor));

    lay->addWidget(lbl);
    lay->addWidget(val);

    return card;
}
