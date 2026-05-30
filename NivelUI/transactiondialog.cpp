#include "transactiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QCompleter>

TransactionDialog::TransactionDialog(const WarehouseManager& depozit, QWidget *parent)
    : QDialog(parent)
    , m_depozit(depozit)
{
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

    // ── Tracking selecție produs ──────────────────────────────────────────────
    // activated(int) se emite când userul alege un item din dropdown sau din popup-ul completer-ului
    connect(comboProduse, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        m_produsSelectat = (index > 0);   // index 0 = placeholder "— selectează produs —"
    });
    // textEdited se emite doar la tastare manuală (nu la setare programatică) → invalidăm selecția
    connect(comboProduse->lineEdit(), &QLineEdit::textEdited, this, [this]() {
        m_produsSelectat = false;
    });
    connect(comboProduse, &QComboBox::currentIndexChanged, this, [this]{
        int index = comboProduse->currentIndex();
        if (index <= 0) {
            spinPret->setValue(0.0);
            return;
        }

        QString idSelectat = comboProduse->currentData().toString();

        const Produs* p = m_depozit.gasesteProdusDupaId(idSelectat);

        if(p!=nullptr)
            spinPret->setValue(p->pret());

    });

    connect(btnConfirm, &QPushButton::clicked, this, [this]() {
        if (valideazaDatele())
            accept();
    });
}

bool TransactionDialog::valideazaDatele()
{
    // Stiluri reutilizabile
    const QString styleEroare = "border: 1.5px solid #dc3545; border-radius: 5px;";
    const QString styleNormal = "";   // QSS-ul global preia controlul

    // ── 1. Produs selectat ────────────────────────────────────────────────────
    // m_produsSelectat devine true doar când userul alege explicit din popup/dropdown
    if (!m_produsSelectat || comboProduse->currentData().toString().isEmpty()) {
        comboProduse->setStyleSheet(styleEroare);
        QMessageBox::warning(this,
                             "Câmp obligatoriu",
                             "Selectează un produs din lista de sugestii.\n"
                             "Poți căuta după nume sau după ID.");
        comboProduse->setFocus();
        return false;
    }
    comboProduse->setStyleSheet(styleNormal);

    // ── 2. Cantitate >= 1 ─────────────────────────────────────────────────────
    if (spinCantitate->value() < 1) {
        spinCantitate->setStyleSheet(styleEroare);
        QMessageBox::warning(this,
                             "Cantitate invalidă",
                             "Cantitatea trebuie să fie de cel puțin 1 bucată.");
        spinCantitate->setFocus();
        return false;
    }
    spinCantitate->setStyleSheet(styleNormal);

    // ── 3. Preț unitar > 0 ────────────────────────────────────────────────────
    if (spinPret->value() <= 0.0) {
        spinPret->setStyleSheet(styleEroare);
        QMessageBox::warning(this,
                             "Preț invalid",
                             "Prețul unitar trebuie să fie mai mare decât 0 RON.");
        spinPret->setFocus();
        return false;
    }
    spinPret->setStyleSheet(styleNormal);

    // ── 4. Companie ne-goală ──────────────────────────────────────────────────
    if (editCompanie->text().trimmed().isEmpty()) {
        editCompanie->setStyleSheet(styleEroare);
        QMessageBox::warning(this,
                             "Câmp obligatoriu",
                             "Introdu numele companiei partenere (furnizor / client).");
        editCompanie->setFocus();
        return false;
    }
    editCompanie->setStyleSheet(styleNormal);

    // ── 5. Stoc suficient (doar la Vânzare) ───────────────────────────────────
    if (m_tipCurent == TipTranzactie::Vanzare) {
        const QString produsId = comboProduse->currentData().toString();
        const Produs *p = m_depozit.gasesteProdusDupaId(produsId);

        if (p && spinCantitate->value() > p->cantitate()) {
            spinCantitate->setStyleSheet(styleEroare);
            QMessageBox::warning(this,
                                 "Stoc insuficient",
                                 QString("Stocul disponibil pentru \"%1\" este %2 buc.\n"
                                         "Nu se poate vinde %3 buc.")
                                     .arg(p->nume())
                                     .arg(p->cantitate())
                                     .arg(spinCantitate->value()));
            spinCantitate->setFocus();
            return false;
        }
        spinCantitate->setStyleSheet(styleNormal);
    }

    return true;   // toate câmpurile sunt valide
}

void TransactionDialog::setupUI(const WarehouseManager& depozit) {
    this->setFixedSize(500, 550);
    this->setStyleSheet("QDialog { background-color: white; }");

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
    btnTabAchizitie->setObjectName("ToggleBtn");
    btnTabVanzare->setObjectName("ToggleBtn");

    btnTabAchizitie->setProperty("active", true);
    btnTabVanzare->setProperty("active", false);

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

    // ── Căutare live după nume sau ID ─────────────────────────────────────────
    comboProduse->setEditable(true);
    comboProduse->setInsertPolicy(QComboBox::NoInsert);  // textul tastat nu devine item nou
    comboProduse->lineEdit()->setPlaceholderText("Caută după nume sau ID...");
    comboProduse->lineEdit()->setClearButtonEnabled(true);

    // Qt creează un QCompleter implicit la setEditable(true) — îl configurăm
    QCompleter *completer = comboProduse->completer();
    completer->setFilterMode(Qt::MatchContains);         // potrivire oriunde în text
    completer->setCaseSensitivity(Qt::CaseInsensitive);  // case-insensitive
    completer->setCompletionMode(QCompleter::PopupCompletion);

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
    btnCancel->setObjectName("BtnTransactionDialogCancel");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnConfirm = new QPushButton("✓ Confirmă achiziție");
    btnConfirm->setObjectName("BtnTransactionDialogSave");
    btnConfirm->setCursor(Qt::PointingHandCursor);

    footer->addWidget(new QLabel("<font color='red'>*</font> obligatoriu"));
    footer->addStretch();
    footer->addWidget(btnCancel);
    footer->addWidget(btnConfirm);
    mainLayout->addLayout(footer);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void TransactionDialog::updateToggleStyle() {

    btnTabAchizitie->setProperty("active", m_tipCurent == TipTranzactie::Achizitionare);
    btnTabVanzare->setProperty("active", m_tipCurent == TipTranzactie::Vanzare);

    // FORȚĂM REÎMPROSPĂTAREA STILULUI (Esential!)
    btnTabAchizitie->style()->unpolish(btnTabAchizitie);
    btnTabAchizitie->style()->polish(btnTabAchizitie);

    btnTabVanzare->style()->unpolish(btnTabVanzare);
    btnTabVanzare->style()->polish(btnTabVanzare);

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
