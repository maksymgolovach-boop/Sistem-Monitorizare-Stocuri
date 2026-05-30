#include "addproductdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QMessageBox>

AddProductDialog::AddProductDialog(QWidget *parent) : QDialog(parent) {
    setupUI();

    // Conectăm butonul de Salvare
    connect(btnSave, &QPushButton::clicked, this, [this]() {
        // Validare de bază (Industry Standard: nu lăsăm userul să salveze un produs fără nume)
        if(editNume->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Eroare de Validare", "Numele este câmp obligatoriu!");
            return;
        }
        if(spinPret->text().isEmpty()){
            return;
        }

        // Dacă totul e ok, închidem dialogul cu semnalul de "Succes" (Accepted)
        accept();
    });

    // Conectăm butonul de Anulare pentru a închide fereastra cu "Eșec/Renunțare" (Rejected)
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AddProductDialog::setupUI() {
    this->setWindowTitle("Adaugă produs nou");
    this->setFixedSize(500, 600); // Dimensiune fixă pentru un aspect modal solid
    this->setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 20);
    mainLayout->setSpacing(20);

    // --- 1. HEADER ---
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *iconLabel = new QLabel("📦");
    iconLabel->setStyleSheet("font-size: 24px; background-color: #e7f1ff; color: #0d6efd; padding: 10px; border-radius: 8px;");

    QVBoxLayout *titleLayout = new QVBoxLayout();
    QLabel *lblTitle = new QLabel("Adaugă produs nou");
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #212529;");
    QLabel *lblSubtitle = new QLabel("Completează câmpurile obligatorii marcate cu *");
    lblSubtitle->setStyleSheet("font-size: 12px; color: #6c757d;");
    titleLayout->addWidget(lblTitle);
    titleLayout->addWidget(lblSubtitle);
    titleLayout->setSpacing(2);

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

    // Linie despărțitoare
    QFrame *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("color: #dee2e6;");
    mainLayout->addWidget(line1);

    // --- 2. BODY (FORMULAR) ---
    QVBoxLayout *formLayout = new QVBoxLayout();
    formLayout->setSpacing(15);

    // ID Produs (Read-only, vizual personalizat)
    generatedId = QUuid::createUuid().toString(QUuid::WithoutBraces); // Generare UUID fără acolade
    QLabel *lblIdTitle = new QLabel("ID produs");
    lblIdTitle->setStyleSheet("font-weight: bold; color: #495057;");

    QFrame *idFrame = new QFrame();
    idFrame->setObjectName("IdFrame");
    QHBoxLayout *idLayout = new QHBoxLayout(idFrame);
    idLayout->setContentsMargins(10, 8, 10, 8);
    idLayout->addWidget(new QLabel("🪪"));
    QLabel *lblIdValue = new QLabel(generatedId);
    lblIdValue->setStyleSheet("color: #6c757d; font-family: monospace;");
    idLayout->addWidget(lblIdValue, 1);
    QLabel *lblAuto = new QLabel("✓ auto-generat");
    lblAuto->setObjectName("AutoBadge");
    idLayout->addWidget(lblAuto);

    QLabel *lblIdHint = new QLabel("Generat automat — nu poate fi modificat manual");
    lblIdHint->setStyleSheet("font-size: 11px; color: #adb5bd;");

    formLayout->addWidget(lblIdTitle);
    formLayout->addWidget(idFrame);
    formLayout->addWidget(lblIdHint);

    // Nume Produs
    QLabel *lblName = new QLabel("Nume produs <font color='#dc3545'>*</font>");
    lblName->setStyleSheet("font-weight: bold; color: #495057;");
    editNume = new QLineEdit();
    editNume->setPlaceholderText("Ex: Lapte UHT 1L");
    editNume->setObjectName("ModernInput");

    formLayout->addWidget(lblName);
    formLayout->addWidget(editNume);

    // Rând cu 2 coloane: Cantitate și Preț
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(15);

    QLabel *lblCant = new QLabel("Cantitate inițială <font color='#dc3545'>*</font>");
    lblCant->setStyleSheet("font-weight: bold; color: #495057;");
    spinCantitate = new QSpinBox();
    spinCantitate->setMaximum(100000);
    spinCantitate->setSuffix(" buc.");
    spinCantitate->setObjectName("ModernInput");

    QLabel *lblPret = new QLabel("Preț unitar <font color='#dc3545'>*</font>");
    lblPret->setStyleSheet("font-weight: bold; color: #495057;");
    spinPret = new QDoubleSpinBox();
    spinPret->setMaximum(100000);
    spinPret->setDecimals(2);
    spinPret->setSuffix(" RON");
    spinPret->setObjectName("ModernInput");

    grid->addWidget(lblCant, 0, 0);
    grid->addWidget(spinCantitate, 1, 0);
    grid->addWidget(lblPret, 0, 1);
    grid->addWidget(spinPret, 1, 1);

    formLayout->addLayout(grid);

    // Prag alertă
    QLabel *lblPrag = new QLabel("Prag alertă stoc <font color='#dc3545'>*</font>");
    lblPrag->setStyleSheet("font-weight: bold; color: #495057;");
    spinPrag = new QSpinBox();
    spinPrag->setMaximum(100000);
    spinPrag->setSuffix(" buc.");
    spinPrag->setObjectName("ModernInput");

    QLabel *lblPragHint = new QLabel("Alertă automată când stocul scade sub această valoare");
    lblPragHint->setStyleSheet("font-size: 11px; color: #adb5bd;");

    formLayout->addWidget(lblPrag);
    formLayout->addWidget(spinPrag);
    formLayout->addWidget(lblPragHint);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // Linie despărțitoare 2
    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("color: #dee2e6;");
    mainLayout->addWidget(line2);

    // --- 3. FOOTER (BUTOANE) ---
    QHBoxLayout *footerLayout = new QHBoxLayout();

    QLabel *lblFooterOblig = new QLabel("<font color='#dc3545'>*</font> câmpuri obligatorii");
    lblFooterOblig->setStyleSheet("font-size: 12px; color: #6c757d;");

    btnCancel = new QPushButton("✕ Anulează");
    btnCancel->setObjectName("BtnDialogCancel");
    btnCancel->setCursor(Qt::PointingHandCursor);

    btnSave = new QPushButton("✓ Salvează produs");
    btnSave->setObjectName("BtnDialogSave");
    btnSave->setCursor(Qt::PointingHandCursor);

    footerLayout->addWidget(lblFooterOblig);
    footerLayout->addStretch();
    footerLayout->addWidget(btnCancel);
    footerLayout->addWidget(btnSave);

    mainLayout->addLayout(footerLayout);
}

ProductData AddProductDialog::getProductData() const {
    // Împachetăm datele și le returnăm
    return {
        generatedId,
        editNume->text().trimmed(),
        spinCantitate->value(),
        spinPret->value(),
        spinPrag->value()
    };
}
