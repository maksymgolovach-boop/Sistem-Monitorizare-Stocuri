#include "transactiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>

TransactionDialog::TransactionDialog(const WarehouseManager& depozit, QWidget *parent) : QDialog(parent) {
    m_generatedId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_currentTimestamp = QDateTime::currentDateTime();

    setupUI(depozit);
    updateToggleStyle();

    // Logică pentru Switch-ul de tip tranzacție
    connect(btnTabAchizitie, &QPushButton::clicked, this, [this]() {
        m_tipCurent = TipTranzactie::Achizitionare;
        btnConfirm->setText("✓ Confirmă achiziție");
        updateToggleStyle();
    });

    connect(btnTabVanzare, &QPushButton::clicked, this, [this]() {
        m_tipCurent = TipTranzactie::Vanzare;
        btnConfirm->setText("✓ Confirmă vânzare");
        updateToggleStyle();
    });

    connect(btnConfirm, &QPushButton::clicked, this, &QDialog::accept);
}

/*
    *comboProduse;
    QSpinBox *spinCantitate;
    QDoubleSpinBox *spinPret;
    QLineEdit *editCompanie;
*/
void TransactionDialog::valideazaDatele(){
    if(comboProduse->currentIndex()==-1)
}

void TransactionDialog::setupUI(const WarehouseManager& depozit) {
    this->setFixedSize(500, 550);
    this->setStyleSheet("background-color: white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 20, 25, 20);
    mainLayout->setSpacing(15);

    // --- 1. HEADER ---
    QHBoxLayout *header = new QHBoxLayout();
    QLabel *icon = new QLabel("📥");
    icon->setStyleSheet("font-size: 22px; background-color: #f8f9fa; padding: 8px; border-radius: 8px;");

    QVBoxLayout *titleText = new QVBoxLayout();
    QLabel *lblTitle = new QLabel("Înregistrare tranzactie");
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    QLabel *lblSub = new QLabel("Intrare stoc - completează detaliile tranzacției");
    lblSub->setStyleSheet("color: #6c757d; font-size: 12px;");
    titleText->addWidget(lblTitle);
    titleText->addWidget(lblSub);

    header->addWidget(icon);
    header->addLayout(titleText);
    header->addStretch();
    mainLayout->addLayout(header);

    // --- 2. TOGGLE ACHIZITIE / VANZARE ---
    QHBoxLayout *toggleLayout = new QHBoxLayout();
    btnTabAchizitie = new QPushButton("⬇ Achiziționare");
    btnTabVanzare = new QPushButton("⬆ Vanzare");
    btnTabAchizitie->setFixedHeight(40);
    btnTabVanzare->setFixedHeight(40);
    toggleLayout->addWidget(btnTabAchizitie);
    toggleLayout->addWidget(btnTabVanzare);
    mainLayout->addLayout(toggleLayout);

    // --- 3. ID & TIMESTAMP ROW ---
    QGridLayout *metaGrid = new QGridLayout();

    QLabel *lblIdT = new QLabel("ID tranzacție");
    lblIdT->setStyleSheet("font-size: 11px; color: #6c757d;");
    QFrame *idFrame = new QFrame();
    idFrame->setStyleSheet("background: #f8f9fa; border: 1px solid #ddd; border-radius: 5px;");
    QHBoxLayout *idL = new QHBoxLayout(idFrame);
    idL->addWidget(new QLabel("🆔 " + m_generatedId.left(18) + "..."));

    QLabel *lblTimeT = new QLabel("Timp");
    lblTimeT->setStyleSheet("font-size: 11px; color: #6c757d;");
    QFrame *timeFrame = new QFrame();
    timeFrame->setStyleSheet("background: #f8f9fa; border: 1px solid #ddd; border-radius: 5px;");
    QHBoxLayout *timeL = new QHBoxLayout(timeFrame);
    timeL->addWidget(new QLabel("🕒 " + m_currentTimestamp.toString("yyyy-MM-dd HH:mm")));

    metaGrid->addWidget(lblIdT, 0, 0);
    metaGrid->addWidget(idFrame, 1, 0);
    metaGrid->addWidget(lblTimeT, 0, 1);
    metaGrid->addWidget(timeFrame, 1, 1);
    mainLayout->addLayout(metaGrid);

    // --- 4. PRODUCT SELECTION ---
    mainLayout->addWidget(new QLabel("Produs <font color='red'>*</font>"));
    comboProduse = new QComboBox();
    comboProduse->setObjectName("ModernInput");

    // Placeholder-ul primește un ID gol ("") ca să știm că nu s-a selectat nimic
    comboProduse->addItem("— selectează produs —", QVariant(""));

    // Iterăm prin unordered_map. pair.first este cheia (QString), pair.second este Produsul
    const auto& hartaProduse = depozit.produse();

    for(auto it = hartaProduse.begin(); it != hartaProduse.end(); ++it) {
        const QString& idProdus = it->first; // Cheia din map
        const Produs& p = it->second;        // Valoarea (obiectul)

        QString textAfisat = p.nume() + " (ID: " + p.id() + ")";

        // Magia Qt: adăugăm textul vizual și ascundem idProdus "în spate"
        comboProduse->addItem(textAfisat, QVariant(idProdus));
    }

    mainLayout->addWidget(comboProduse);

    // --- 5. CANTITATE & PRET ---
    QGridLayout *amountGrid = new QGridLayout();
    spinCantitate = new QSpinBox();
    spinCantitate->setSuffix(" buc.");
    spinCantitate->setObjectName("ModernInput");

    spinPret = new QDoubleSpinBox();
    spinPret->setSuffix(" RON");
    spinPret->setObjectName("ModernInput");

    amountGrid->addWidget(new QLabel("Cantitate <font color='red'>*</font>"), 0, 0);
    amountGrid->addWidget(spinCantitate, 1, 0);
    amountGrid->addWidget(new QLabel("Preț unitar <font color='red'>*</font>"), 0, 1);
    amountGrid->addWidget(spinPret, 1, 1);
    mainLayout->addLayout(amountGrid);

    // --- 6. COMPANIE ---
    mainLayout->addWidget(new QLabel("Compania parteneră <font color='red'>*</font>"));
    editCompanie = new QLineEdit();
    editCompanie->setPlaceholderText("Ex: Lactalis SRL");
    editCompanie->setObjectName("ModernInput");
    mainLayout->addWidget(editCompanie);

    mainLayout->addStretch();

    // --- 7. FOOTER ---
    QHBoxLayout *footer = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("✕ Anulează");
    btnCancel->setObjectName("BtnDialogCancel");
    btnConfirm = new QPushButton("✓ Confirmă achiziție");
    btnConfirm->setObjectName("BtnDialogSave");

    footer->addWidget(new QLabel("<font color='red'>*</font> obligatoriu"));
    footer->addStretch();
    footer->addWidget(btnCancel);
    footer->addWidget(btnConfirm);
    mainLayout->addLayout(footer);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void TransactionDialog::updateToggleStyle() {
    QString active = "background-color: white; border: 2px solid #000; border-radius: 8px; font-weight: bold;";
    QString inactive = "background-color: #f8f9fa; border: 1px solid #ddd; border-radius: 8px; color: #666;";

    btnTabAchizitie->setStyleSheet(m_tipCurent == TipTranzactie::Achizitionare ? active : inactive);
    btnTabVanzare->setStyleSheet(m_tipCurent == TipTranzactie::Vanzare ? active : inactive);
}

TransactionData TransactionDialog::getTransactionData() const {
    return {
        m_generatedId,
        m_currentTimestamp,
        m_tipCurent,
        comboProduse->currentData().toString(),
        spinCantitate->value(),
        spinPret->value(),
        editCompanie->text()
    };
}
