#include "editproductdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QMessageBox>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

EditProductDialog::EditProductDialog(const Produs &produs, QWidget *parent)
    : QDialog(parent)
    , m_produsOriginal(produs)
{
    setupUI();
    populateCampuri();

    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (editNume->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Eroare de validare",
                                 "Numele produsului este câmp obligatoriu!");
            editNume->setFocus();
            return;
        }
        if (spinPret->value() < 0.0) {
            QMessageBox::warning(this, "Eroare de validare",
                                 "Prețul nu poate fi negativ!");
            spinPret->setFocus();
            return;
        }
        accept();
    });

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

// ─────────────────────────────────────────────────────────────────────────────
// getProdusCuModificari
// ─────────────────────────────────────────────────────────────────────────────

Produs EditProductDialog::getProdusCuModificari() const
{
    // Pornim de la copia originală — ID-ul rămâne identic
    Produs actualizat = m_produsOriginal;

    actualizat.setNume      (editNume->text().trimmed());
    actualizat.setCantitate (spinCantitate->value());
    actualizat.setPret      (spinPret->value());
    actualizat.setPragAlerta(spinPrag->value());

    return actualizat;
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUI — construiește interfața (același stil ca AddProductDialog)
// ─────────────────────────────────────────────────────────────────────────────

void EditProductDialog::setupUI()
{
    this->setWindowTitle("Modifică produs");
    this->setFixedSize(500, 600);
    this->setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 20);
    mainLayout->setSpacing(20);

    // ── 1. HEADER ─────────────────────────────────────────────────────────────

    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *iconLabel = new QLabel("✏️");
    iconLabel->setStyleSheet(
        "font-size: 24px; background-color: #fff3cd; "
        "color: #856404; padding: 10px; border-radius: 8px;");

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    QLabel *lblTitle = new QLabel("Modifică produs");
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #212529;");
    QLabel *lblSubtitle = new QLabel("Editează câmpurile dorite, apoi salvează modificările");
    lblSubtitle->setStyleSheet("font-size: 12px; color: #6c757d;");
    titleLayout->addWidget(lblTitle);
    titleLayout->addWidget(lblSubtitle);

    QPushButton *btnCloseTop = new QPushButton("✕");
    btnCloseTop->setFixedSize(30, 30);
    btnCloseTop->setObjectName("BtnCloseTop");
    connect(btnCloseTop, &QPushButton::clicked, this, &QDialog::reject);

    headerLayout->addWidget(iconLabel);
    headerLayout->addSpacing(10);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(btnCloseTop);
    mainLayout->addLayout(headerLayout);

    // Separator
    QFrame *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("color: #dee2e6;");
    mainLayout->addWidget(line1);

    // ── 2. FORMULAR ───────────────────────────────────────────────────────────

    QVBoxLayout *formLayout = new QVBoxLayout();
    formLayout->setSpacing(15);

    // ── ID produs (read-only) ──────────────────────────────────────────────────
    QLabel *lblIdTitle = new QLabel("ID produs");
    lblIdTitle->setStyleSheet("font-weight: bold; color: #495057;");

    QFrame *idFrame = new QFrame();
    idFrame->setObjectName("IdFrame");
    QHBoxLayout *idLayout = new QHBoxLayout(idFrame);
    idLayout->setContentsMargins(10, 8, 10, 8);
    idLayout->addWidget(new QLabel("🪪"));
    lblIdValue = new QLabel();  // populat în populateCampuri()
    lblIdValue->setStyleSheet("color: #6c757d; font-family: monospace;");
    idLayout->addWidget(lblIdValue, 1);

    QLabel *lblReadOnly = new QLabel("🔒 read-only");
    lblReadOnly->setStyleSheet(
        "font-size: 11px; background-color: #f8f9fa; "
        "color: #6c757d; padding: 2px 6px; border-radius: 4px;");
    idLayout->addWidget(lblReadOnly);

    QLabel *lblIdHint = new QLabel("ID-ul produsului nu poate fi modificat");
    lblIdHint->setStyleSheet("font-size: 11px; color: #adb5bd;");

    formLayout->addWidget(lblIdTitle);
    formLayout->addWidget(idFrame);
    formLayout->addWidget(lblIdHint);

    // ── Nume produs ────────────────────────────────────────────────────────────
    QLabel *lblName = new QLabel("Nume produs <font color='#dc3545'>*</font>");
    lblName->setStyleSheet("font-weight: bold; color: #495057;");
    editNume = new QLineEdit();
    editNume->setObjectName("ModernInput");

    formLayout->addWidget(lblName);
    formLayout->addWidget(editNume);

    // ── Cantitate + Preț (2 coloane) ───────────────────────────────────────────
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(15);

    QLabel *lblCant = new QLabel("Cantitate <font color='#dc3545'>*</font>");
    lblCant->setStyleSheet("font-weight: bold; color: #495057;");
    spinCantitate = new QSpinBox();
    spinCantitate->setMaximum(999999);
    spinCantitate->setSuffix(" buc.");
    spinCantitate->setObjectName("ModernInput");

    QLabel *lblPret = new QLabel("Preț unitar <font color='#dc3545'>*</font>");
    lblPret->setStyleSheet("font-weight: bold; color: #495057;");
    spinPret = new QDoubleSpinBox();
    spinPret->setMaximum(999999.99);
    spinPret->setDecimals(2);
    spinPret->setSuffix(" RON");
    spinPret->setObjectName("ModernInput");

    grid->addWidget(lblCant, 0, 0);
    grid->addWidget(spinCantitate, 1, 0);
    grid->addWidget(lblPret, 0, 1);
    grid->addWidget(spinPret, 1, 1);
    formLayout->addLayout(grid);

    // ── Prag alertă ────────────────────────────────────────────────────────────
    QLabel *lblPrag = new QLabel("Prag alertă stoc <font color='#dc3545'>*</font>");
    lblPrag->setStyleSheet("font-weight: bold; color: #495057;");
    spinPrag = new QSpinBox();
    spinPrag->setMaximum(999999);
    spinPrag->setSuffix(" buc.");
    spinPrag->setObjectName("ModernInput");

    QLabel *lblPragHint = new QLabel(
        "Alertă automată când stocul scade sub această valoare");
    lblPragHint->setStyleSheet("font-size: 11px; color: #adb5bd;");

    formLayout->addWidget(lblPrag);
    formLayout->addWidget(spinPrag);
    formLayout->addWidget(lblPragHint);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // Separator
    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("color: #dee2e6;");
    mainLayout->addWidget(line2);

    // ── 3. FOOTER ─────────────────────────────────────────────────────────────

    QHBoxLayout *footerLayout = new QHBoxLayout();

    QLabel *lblOblig = new QLabel("<font color='#dc3545'>*</font> câmpuri obligatorii");
    lblOblig->setStyleSheet("font-size: 12px; color: #6c757d;");

    btnCancel = new QPushButton("✕ Anulează");
    btnCancel->setObjectName("BtnDialogCancel");
    btnCancel->setCursor(Qt::PointingHandCursor);

    btnSave = new QPushButton("✓ Salvează modificările");
    btnSave->setObjectName("BtnDialogSave");
    btnSave->setCursor(Qt::PointingHandCursor);

    footerLayout->addWidget(lblOblig);
    footerLayout->addStretch();
    footerLayout->addWidget(btnCancel);
    footerLayout->addWidget(btnSave);
    mainLayout->addLayout(footerLayout);
}

// ─────────────────────────────────────────────────────────────────────────────
// populateCampuri — pre-completează câmpurile cu datele produsului original
// ─────────────────────────────────────────────────────────────────────────────

void EditProductDialog::populateCampuri()
{
    lblIdValue->setText(m_produsOriginal.id());
    editNume->setText(m_produsOriginal.nume());
    spinCantitate->setValue(m_produsOriginal.cantitate());
    spinPret->setValue(m_produsOriginal.pret());
    spinPrag->setValue(m_produsOriginal.pragAlerta());
}
