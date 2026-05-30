#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

// ─────────────────────────────────────────────────────────────────────────────

SettingsDialog::SettingsDialog(const SettingsData &current, QWidget *parent)
    : QDialog(parent)
{
    setupUI(current);

    // ── Browse produse ────────────────────────────────────────────────────────
    connect(m_btnBrowseDepozit, &QPushButton::clicked, this, [this]() {
        const QString cale = QFileDialog::getSaveFileName(
            this,
            "Selectează fișierul pentru catalog produse",
            m_editCaleDepozit->text().isEmpty()
                ? QDir::homePath() : m_editCaleDepozit->text(),
            "Fișiere JSON (*.json);;Toate fișierele (*)");
        if (!cale.isEmpty())
            m_editCaleDepozit->setText(cale);
    });

    // ── Browse tranzacții ─────────────────────────────────────────────────────
    connect(m_btnBrowseTranzactii, &QPushButton::clicked, this, [this]() {
        const QString cale = QFileDialog::getSaveFileName(
            this,
            "Selectează fișierul pentru istoric tranzacții",
            m_editCaleTranzactii->text().isEmpty()
                ? QDir::homePath() : m_editCaleTranzactii->text(),
            "Fișiere JSON (*.json);;Toate fișierele (*)");
        if (!cale.isEmpty())
            m_editCaleTranzactii->setText(cale);
    });

    connect(m_btnSave,   &QPushButton::clicked, this, [this]() {
        if (valideaza()) accept();
    });
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

// ─────────────────────────────────────────────────────────────────────────────

SettingsData SettingsDialog::getSettings() const
{
    return {
        m_editNume->text().trimmed(),
        m_editCaleDepozit->text().trimmed(),
        m_editCaleTranzactii->text().trimmed()
    };
}

// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupUI(const SettingsData &current)
{
    setWindowTitle("Setări aplicație");
    setFixedSize(540, 430);
    setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    // ── HEADER ────────────────────────────────────────────────────────────────
    QHBoxLayout *hdr = new QHBoxLayout();
    QLabel *icon = new QLabel("⚙");
    icon->setStyleSheet(
        "font-size: 22px; background:#f8f9fa; padding:8px;"
        "border-radius:8px;");
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);

    QVBoxLayout *titleCol = new QVBoxLayout();
    titleCol->setSpacing(2);
    QLabel *lblTitle = new QLabel("Setări aplicație");
    lblTitle->setStyleSheet("font-size:17px; font-weight:bold; color:#212529;");
    QLabel *lblSub = new QLabel("Configurează identitatea și locațiile fișierelor de date");
    lblSub->setStyleSheet("font-size:11px; color:#6c757d;");
    titleCol->addWidget(lblTitle);
    titleCol->addWidget(lblSub);

    hdr->addWidget(icon);
    hdr->addSpacing(10);
    hdr->addLayout(titleCol);
    hdr->addStretch();
    root->addLayout(hdr);

    // separator
    auto makeSep = [&]() {
        QFrame *f = new QFrame();
        f->setFrameShape(QFrame::HLine);
        f->setStyleSheet("color:#dee2e6;");
        return f;
    };
    root->addWidget(makeSep());

    // ── SECȚIUNEA IDENTITATE ──────────────────────────────────────────────────
    QLabel *lblSecId = new QLabel("Identitate");
    lblSecId->setStyleSheet("font-weight:bold; color:#495057; font-size:12px;");
    root->addWidget(lblSecId);

    QLabel *lblNume = new QLabel("Nume depozit <font color='#dc3545'>*</font>");
    lblNume->setStyleSheet("color:#495057;");
    m_editNume = new QLineEdit(current.numeDepozit);
    m_editNume->setPlaceholderText("Ex: Depozit Central");
    m_editNume->setObjectName("ModernInput");
    m_editNume->setMinimumHeight(34);
    root->addWidget(lblNume);
    root->addWidget(m_editNume);

    root->addWidget(makeSep());

    // ── SECȚIUNEA FIȘIERE DE DATE ────────────────────────────────────────────
    QLabel *lblSecFis = new QLabel("Fișiere de date");
    lblSecFis->setStyleSheet("font-weight:bold; color:#495057; font-size:12px;");
    root->addWidget(lblSecFis);

    // -- Catalog produse
    QLabel *lblDepozit = new QLabel("📦  Catalog produse (depozit.json)");
    lblDepozit->setStyleSheet("color:#495057;");
    root->addWidget(lblDepozit);

    QHBoxLayout *rowDepozit = new QHBoxLayout();
    m_editCaleDepozit = new QLineEdit(current.caleDepozit);
    m_editCaleDepozit->setObjectName("ModernInput");
    m_editCaleDepozit->setMinimumHeight(34);
    m_editCaleDepozit->setReadOnly(true);
    m_editCaleDepozit->setStyleSheet(
        "border:1px solid #ced4da; border-radius:6px; padding:6px 10px;"
        "font-size:12px; color:#495057; background:#f8f9fa;");

    m_btnBrowseDepozit = new QPushButton("📂  Browse");
    m_btnBrowseDepozit->setObjectName("BtnFilter");
    m_btnBrowseDepozit->setMinimumHeight(34);
    m_btnBrowseDepozit->setFixedWidth(100);
    m_btnBrowseDepozit->setCursor(Qt::PointingHandCursor);

    rowDepozit->addWidget(m_editCaleDepozit);
    rowDepozit->addWidget(m_btnBrowseDepozit);
    root->addLayout(rowDepozit);

    // -- Tranzacții
    QLabel *lblTranz = new QLabel("🔄  Istoric tranzacții (tranzactii.json)");
    lblTranz->setStyleSheet("color:#495057;");
    root->addWidget(lblTranz);

    QHBoxLayout *rowTranz = new QHBoxLayout();
    m_editCaleTranzactii = new QLineEdit(current.caleTranzactii);
    m_editCaleTranzactii->setObjectName("ModernInput");
    m_editCaleTranzactii->setMinimumHeight(34);
    m_editCaleTranzactii->setReadOnly(true);
    m_editCaleTranzactii->setStyleSheet(
        "border:1px solid #ced4da; border-radius:6px; padding:6px 10px;"
        "font-size:12px; color:#495057; background:#f8f9fa;");

    m_btnBrowseTranzactii = new QPushButton("📂  Browse");
    m_btnBrowseTranzactii->setObjectName("BtnFilter");
    m_btnBrowseTranzactii->setMinimumHeight(34);
    m_btnBrowseTranzactii->setFixedWidth(100);
    m_btnBrowseTranzactii->setCursor(Qt::PointingHandCursor);

    rowTranz->addWidget(m_editCaleTranzactii);
    rowTranz->addWidget(m_btnBrowseTranzactii);
    root->addLayout(rowTranz);

    // ── Info banner ───────────────────────────────────────────────────────────
    QLabel *lblInfo = new QLabel(
        "ℹ  Dacă selectezi un fișier existent, datele din el vor fi încărcate imediat.\n"
        "    Dacă selectezi o locație nouă, datele curente vor fi salvate acolo la operația următoare.");
    lblInfo->setStyleSheet(
        "background:#f0f4ff; border:1px solid #b6d4fe; border-radius:6px;"
        "color:#0d6efd; font-size:10.5px; padding:8px 10px;");
    lblInfo->setWordWrap(true);
    root->addWidget(lblInfo);

    root->addStretch();

    // ── FOOTER ────────────────────────────────────────────────────────────────
    root->addWidget(makeSep());

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addWidget(new QLabel("<font color='#dc3545'>*</font> câmp obligatoriu"));
    footer->addStretch();

    m_btnCancel = new QPushButton("✕  Anulează");
    m_btnCancel->setObjectName("BtnDialogCancel");
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    m_btnSave = new QPushButton("✓  Salvează setările");
    m_btnSave->setObjectName("BtnDialogSave");
    m_btnSave->setCursor(Qt::PointingHandCursor);

    footer->addWidget(m_btnCancel);
    footer->addWidget(m_btnSave);
    root->addLayout(footer);
}

// ─────────────────────────────────────────────────────────────────────────────

bool SettingsDialog::valideaza()
{
    if (m_editNume->text().trimmed().isEmpty()) {
        m_editNume->setStyleSheet("border:1.5px solid #dc3545; border-radius:6px; padding:6px;");
        QMessageBox::warning(this, "Câmp obligatoriu",
                             "Numele depozitului nu poate fi gol.");
        m_editNume->setFocus();
        return false;
    }
    m_editNume->setStyleSheet("");

    if (m_editCaleDepozit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Câmp obligatoriu",
                             "Selectează o cale pentru fișierul de produse.");
        return false;
    }
    if (m_editCaleTranzactii->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Câmp obligatoriu",
                             "Selectează o cale pentru fișierul de tranzacții.");
        return false;
    }
    return true;
}
