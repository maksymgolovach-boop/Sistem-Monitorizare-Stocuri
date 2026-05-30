#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QStandardPaths>
#include <QSettings>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <algorithm>
#include "../NivelUI/addproductdialog.h"
#include "../NivelUI/transactiondialog.h"
#include "../NivelUI/editproductdialog.h"
#include "../NivelUI/productdetailsdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_storagedepozit()
    , m_storagetranzactii()
    , depozit(&m_storagedepozit)
    , istoric(&m_storagetranzactii)
{
    ui->setupUi(this);
    QSettings settings;
    m_storagedepozit.setCaleFisier(
        settings.value("stocaredepozit/cale",
                       QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                           + "/SistemMonitorizareStocuri/depozit.json").toString());

    m_storagetranzactii.setCaleFisier(
        settings.value("stocaretranzactii/cale",
                       QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                           + "/SistemMonitorizareStocuri/tranzactii.json").toString());

    if(m_storagedepozit.exista())
        depozit.incarcaDate();
    if(m_storagetranzactii.exista())
        istoric.incarcaDate();
    setupLayout();
}

void MainWindow::setupLayout() {
    // 1. Widget-ul central și Layout-ul Principal (Vertical)
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- ZONA 1: HEADER (Sus, pe toată lățimea) ---
    header = new QWidget();
    header->setFixedHeight(50);
    header->setObjectName("Header");

    mainLayout->addWidget(header);

    // --- ZONA 2: Zona de conținut de sub Header (Orizontală) ---
    QWidget *contentArea = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    mainLayout->addWidget(contentArea); // Adăugăm zona de conținut în layout-ul principal

    // --- ZONA 2.1: SIDEBAR (În stânga, sub header) ---
    sidebar = new QWidget();
    sidebar->setFixedWidth(230);
    sidebar->setObjectName("Sidebar");
    setupSidebar();
    contentLayout->addWidget(sidebar);

    // --- ZONA 2.2: MAIN PANEL (În dreapta sidebar-ului) ---
    mainPanel = new QWidget();
    mainPanel->setObjectName("MainPanel");
    QVBoxLayout *panelLayout = new QVBoxLayout(mainPanel);
    contentLayout->addWidget(mainPanel);

    // Configurare StackedWidget în interiorul Main Panel
    stackedWidget = new QStackedWidget();
    panelLayout->addWidget(stackedWidget);

    QWidget *dashboardPage = new QWidget();
    dashboardPage->setObjectName("PageContent");
    setupDashboardPage(dashboardPage);

    QWidget *productsPage = new QWidget();
    productsPage->setObjectName("PageContent");
    setupProductsPage(productsPage); // Inițializăm noua pagină

    QWidget *alertsPage = new QWidget();
    alertsPage->setObjectName("PageContent");
    setupAlertsPage(alertsPage);

    QWidget *transactionsPage = new QWidget();
    transactionsPage->setObjectName("PageContent");
    setupHistoryPage(transactionsPage);

    // Adăugăm paginile
    stackedWidget->addWidget(dashboardPage);        // Index 0
    stackedWidget->addWidget(productsPage);         // Index 1
    stackedWidget->addWidget(alertsPage);           // Index 2

    stackedWidget->addWidget(transactionsPage);     // Index 3
}

void MainWindow::setupSidebar() {
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(10, 20, 10, 20);
    sideLayout->setSpacing(5);

    // --- SECTIUNEA PRINCIPAL ---
    QLabel *lblPrincipal = new QLabel("PRINCIPAL");
    sideLayout->addWidget(lblPrincipal);

    btnDashboard = new QPushButton("Meniu Principal");
    btnProducts = new QPushButton("Produse");
    btnAlerts = new QPushButton("Alerte");

    sideLayout->addWidget(btnDashboard);
    sideLayout->addWidget(btnProducts);
    sideLayout->addWidget(btnAlerts);

    sideLayout->addSpacing(20); // Spațiu între secțiuni

    // --- SECTIUNEA TRANZACTII ---
    QLabel *lblTranzactii = new QLabel("TRANZACTII");
    sideLayout->addWidget(lblTranzactii);

    btnSales = new QPushButton("Tranzactie noua");
    btnHistory = new QPushButton("Istoric Tranzactii");

    sideLayout->addWidget(btnSales);
    sideLayout->addWidget(btnHistory);

    connect(btnDashboard, &QPushButton::clicked, this, [this]() { stackedWidget->setCurrentIndex(0); });
    connect(btnProducts, &QPushButton::clicked, this, [this]() { stackedWidget->setCurrentIndex(1); });
    connect(btnAlerts, &QPushButton::clicked, this, [this]() { stackedWidget->setCurrentIndex(2); });
    connect(btnHistory, &QPushButton::clicked, this, [this]() {stackedWidget->setCurrentIndex(3);});

    connect(btnSales, &QPushButton::clicked, this, &MainWindow::onBtnSalesClicked);

    sideLayout->addStretch(); // Împinge totul în sus (Industry Standard)

}

void MainWindow::setupDashboardPage(QWidget *page){
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    // --- 1. SECTIUNEA CARDURI (Orizontal) ---
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(15);

    QString produseTotalelbl = QString::number(depozit.numarProduse());
    QString valoareDepozit   = QString::number(depozit.ValoareProduse(), 'f', 2);

    cardsLayout->addWidget(createStatCard("Produse Totale", produseTotalelbl, "CardTotal"));
    cardsLayout->addWidget(createStatCard("Valoare Stoc",   valoareDepozit,   "CardValue"));

    // Capturăm labelul de valoare din cardul "Sub Prag Alertă" pentru a-i schimba culoarea
    QWidget *alertCard = createStatCard("Sub Prag Alertă", "0", "CardAlert");
    dashboardAlertValue = alertCard->findChild<QLabel*>("StatValue");
    cardsLayout->addWidget(alertCard);

    // Capturăm labelul de valoare din cardul "Tranzacții" pentru actualizare live
    QWidget *transCard = createStatCard("Tranzacții", "0", "CardTrans");
    dashboardTransValue = transCard->findChild<QLabel*>("StatValue");
    cardsLayout->addWidget(transCard);

    layout->addLayout(cardsLayout);

    // --- 2. BANNER ALERTA ---
    QLabel *lblAlertBanner = new QLabel();
    lblAlertBanner->setObjectName("AlertBanner");
    lblAlertBanner->setFixedHeight(40);
    lblAlertBanner->setAlignment(Qt::AlignCenter);
    lblAlertBanner->setVisible(false);   // populateDashboard() decide vizibilitatea
    layout->addWidget(lblAlertBanner);

    // --- 3. LISTA PRODUSE (Tabel) — top 15 după cantitate ---
    QLabel *lblTableTitle = new QLabel("Top 15 produse după stoc");
    lblTableTitle->setObjectName("TableLabel");
    layout->addWidget(lblTableTitle);

    dashboardTable = new QTableWidget();
    dashboardTable->setColumnCount(4);
    dashboardTable->setHorizontalHeaderLabels({"Produs", "Preț (RON)", "Stoc", "Status"});
    dashboardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dashboardTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dashboardTable->setFrameShape(QFrame::NoFrame);
    dashboardTable->setShowGrid(false);
    dashboardTable->verticalHeader()->setVisible(false);
    dashboardTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dashboardTable->setObjectName("DashboardTable");
    layout->addWidget(dashboardTable);

    // Populăm imediat cu datele existente
    populateDashboard();

    // Double-click → detalii produs (ID stocat în Qt::UserRole pe col 0)
    connect(dashboardTable, &QTableWidget::cellDoubleClicked,
            this, [this](int row, int) {
        if (auto *item = dashboardTable->item(row, 0))
            showProductDetails(item->data(Qt::UserRole).toString());
    });
}

QWidget* MainWindow::createStatCard(QString title, QString value, QString objectName) {
    QWidget *card = new QWidget();
    card->setObjectName("StatCard"); // Stilul general de card
    card->setProperty("type", objectName); // Opțional, pentru culori specifice fiecărui card

    QVBoxLayout *layout = new QVBoxLayout(card);

    QLabel *lblTitle = new QLabel(title);
    lblTitle->setObjectName("StatTitle");

    QLabel *lblValue = new QLabel(value);
    lblValue->setObjectName("StatValue");

    layout->addWidget(lblTitle);
    layout->addWidget(lblValue);

    return card;
}

void MainWindow::populateDashboard()
{
    if (!dashboardAlertValue || !dashboardTransValue || !dashboardTable)
        return;

    // ── 1. Card "Tranzacții" ──────────────────────────────────────────────────
    dashboardTransValue->setText(QString::number(istoric.numar()));

    // ── 2. Card "Sub Prag Alertă" — valoare + culoare roșu/verde ─────────────
    const int nrAlerte = static_cast<int>(depozit.produseSubPrag().size());
    dashboardAlertValue->setText(QString::number(nrAlerte));
    if (nrAlerte > 0)
        dashboardAlertValue->setStyleSheet("color: #dc3545; font-weight: bold;");  // roșu
    else
        dashboardAlertValue->setStyleSheet("color: #198754; font-weight: bold;");  // verde

    // ── 3. Tabel — top 15 produse după cantitate descrescătoare ──────────────
    const auto &harta = depozit.produse();

    // Colectăm pointeri, sortăm, tăiem la 15
    std::vector<const Produs*> lista;
    lista.reserve(harta.size());
    for (const auto &pereche : harta)
        lista.push_back(&pereche.second);

    std::sort(lista.begin(), lista.end(), [](const Produs *a, const Produs *b) {
        return a->cantitate() > b->cantitate();
    });
    if (lista.size() > 15)
        lista.resize(15);

    dashboardTable->setRowCount(0);
    for (const Produs *p : lista) {
        int row = dashboardTable->rowCount();
        dashboardTable->insertRow(row);

        // Stocăm ID-ul în UserRole pe primul item — necesar pentru double-click → detalii
        QTableWidgetItem *numeItem = new QTableWidgetItem(p->nume());
        numeItem->setData(Qt::UserRole, p->id());
        dashboardTable->setItem(row, 0, numeItem);
        dashboardTable->setItem(row, 1, new QTableWidgetItem(
            QString::number(p->pret(), 'f', 2)));
        dashboardTable->setItem(row, 2, new QTableWidgetItem(
            QString::number(p->cantitate())));

        // Status cu culoare
        QTableWidgetItem *statusItem;
        if (p->esteSubPrag()) {
            statusItem = new QTableWidgetItem("⚠ Sub Prag");
            statusItem->setForeground(QBrush(QColor("#dc3545")));
        } else {
            statusItem = new QTableWidgetItem("✓ OK");
            statusItem->setForeground(QBrush(QColor("#198754")));
        }
        statusItem->setFont(QFont("Arial", 9, QFont::Bold));
        statusItem->setTextAlignment(Qt::AlignCenter);
        dashboardTable->setItem(row, 3, statusItem);
    }
}

void MainWindow::setupProductsPage(QWidget* page){
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // --- 1. BARA DE SUS (Controale, Căutare și Acțiuni) ---
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->setSpacing(10);

    // Caseta de căutare (Textbar)
    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Caută un produs după nume");
    searchBar->setObjectName("SearchBar");
    searchBar->setMinimumHeight(35);
    searchBar->setClearButtonEnabled(true);

    // Butonul de editare
    btnEdit = new QPushButton("✎ Editează");
    btnEdit->setObjectName("BtnFilter");
    btnEdit->setMinimumHeight(35);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setEnabled(false);   // activ doar când e selectat un rând

    // ComboBox de sortare
    comboFilterProducts = new QComboBox();
    comboFilterProducts->setObjectName("BtnFilter");
    comboFilterProducts->setMinimumHeight(35);
    comboFilterProducts->setCursor(Qt::PointingHandCursor);
    comboFilterProducts->addItem("▼ Sortare implicită");        // index 0 → Default
    comboFilterProducts->addItem("Preț ↑ Crescător");           // index 1 → PretAsc
    comboFilterProducts->addItem("Preț ↓ Descrescător");        // index 2 → PretDesc
    comboFilterProducts->addItem("Cantitate ↑ Crescătoare");    // index 3 → CantiCresc
    comboFilterProducts->addItem("Cantitate ↓ Descrescătoare"); // index 4 → CantiDesc
    comboFilterProducts->addItem("Nr. Tranzacții ↓");           // index 5 → NrTranzactii

    // Butonul Adaugă Produs
    btnAddProduct = new QPushButton("+ Adaugă Produs");
    btnAddProduct->setObjectName("BtnAdd");
    btnAddProduct->setMinimumHeight(35);
    btnAddProduct->setCursor(Qt::PointingHandCursor);

    // Butonul Șterge Produs
    btnDeleteProduct = new QPushButton("- Șterge Produs");
    btnDeleteProduct->setObjectName("BtnDelete");
    btnDeleteProduct->setMinimumHeight(35);
    btnDeleteProduct->setCursor(Qt::PointingHandCursor);
    btnDeleteProduct->setEnabled(false);

    // Asamblăm bara de sus
    topBarLayout->addWidget(searchBar, 4);
    topBarLayout->addSpacing(10);
    topBarLayout->addWidget(btnAddProduct, 1);
    topBarLayout->addWidget(comboFilterProducts, 2);
    topBarLayout->addWidget(btnEdit, 1);
    topBarLayout->addWidget(btnDeleteProduct, 1);

    mainLayout->addLayout(topBarLayout);

    // --- 2. TABELUL PRINCIPAL DE PRODUSE ---
    productsTable = new QTableWidget();
    productsTable->setObjectName("MainProductsTable");

    productsTable->setColumnCount(5);
    productsTable->setHorizontalHeaderLabels({"ID", "Nume Produs", "Stoc", "Preț (RON)", "Nr. Tranzacții"});
    productsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Comportament profesional pentru tabel (la fel ca la dashboard)
    productsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsTable->setSelectionMode(QAbstractItemView::SingleSelection); // Permitem ștergerea unui singur rând odată
    productsTable->verticalHeader()->setVisible(false);
    productsTable->setShowGrid(false);
    productsTable->setFrameShape(QFrame::NoFrame);
    productsTable->itemSelectionChanged();

    mainLayout->addWidget(productsTable);

    // --- 3. POPULARE DATE ---
    populateProductsTable();

    // Double-click → detalii produs (col 0 = ID)
    connect(productsTable, &QTableWidget::cellDoubleClicked,
            this, [this](int row, int) {
        if (auto *item = productsTable->item(row, 0))
            showProductDetails(item->text());
    });

    //Add button
    connect(btnAddProduct, &QPushButton::clicked, this, [this]() {
        AddProductDialog dialog(this);

        // 2. Afișăm fereastra și așteptăm ca utilizatorul să o închidă
        if (dialog.exec() == QDialog::Accepted) {

            // 3. Extragem datele introduse
            ProductData dateNoi = dialog.getProductData();

            Produs nouprodus(dateNoi.nume,dateNoi.pret,dateNoi.cantitate, dateNoi.PragAlerta);
            depozit.adaugaProdus(nouprodus);
            m_storagedepozit.salveaza(depozit.produse());
            UpdateUI();
        }
    });

    connect(productsTable, &QTableWidget::itemSelectionChanged, this,[this](){
        bool isRowSelected = productsTable->selectionModel()->hasSelection();
        btnDeleteProduct->setEnabled(isRowSelected);
        btnEdit->setEnabled(isRowSelected);
    });

    // Sortare produse
    connect(comboFilterProducts, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        static const SortProduse mapare[] = {
            SortProduse::Default,
            SortProduse::PretAsc,   SortProduse::PretDesc,
            SortProduse::CantiCresc, SortProduse::CantiDesc,
            SortProduse::NrTranzactii
        };
        m_sortProduse = mapare[index];
        populateProductsTable();
    });

    connect(btnEdit, &QPushButton::clicked,this,[this](){
        int Product = productsTable->currentRow();
        const Produs *produsPtr = depozit.gasesteProdusDupaId(productsTable->item(Product, 0)->text());

        if (!produsPtr) {
            QMessageBox::critical(this, "Eroare", "Produsul selectat nu mai există în depozit.");
            return;
        }
        const Produs produsSnapshot = *produsPtr;   // copie — payload pentru tranzacție


        EditProductDialog dialog(produsSnapshot, this);

        if(dialog.exec() == QDialog::Accepted){
            const Produs &produsEditat = dialog.getProdusCuModificari();

            depozit.actualizeazaProdus(produsEditat);

            m_storagedepozit.salveaza(depozit.produse());
            UpdateUI();
        }
    });

    //deletebutton
    connect(btnDeleteProduct, &QPushButton::clicked, this, [this](){
        int Product = productsTable->currentRow();
        QString IDprodus = productsTable->item(Product, 0)->text();
        QString nume = productsTable->item(Product, 1)->text();

        QString textAccept = QString("Confirmati stergerea produsului cu numele '%1'\nSi id-ul: %2").arg(nume, IDprodus);
        QMessageBox ConfirmDelete(this);
        ConfirmDelete.setWindowTitle("Confirma stergerea");
        ConfirmDelete.setText(textAccept);
        ConfirmDelete.setInformativeText("Această acțiune este ireversibilă și va actualiza stocul total.");
        ConfirmDelete.setIcon(QMessageBox::Warning);
        QPushButton *btnConfirmDelete = ConfirmDelete.addButton("Sterge", QMessageBox::DestructiveRole);
        QPushButton *btnCancel = ConfirmDelete.addButton("Anulează", QMessageBox::RejectRole);
        ConfirmDelete.setDefaultButton(btnCancel);
        ConfirmDelete.exec();

        if (ConfirmDelete.clickedButton() == btnConfirmDelete) {
            depozit.eliminaProdus(IDprodus);
            m_storagedepozit.salveaza(depozit.produse());
            UpdateUI();
        }
    });

    // Search Bar — delegăm la metoda reutilizabilă
    connect(searchBar, &QLineEdit::textChanged, this, [this]() { applyProductsSearch(); });
}

void MainWindow::populateProductsTable()
{
    // ── 1. Construim lista de copii pentru sortare ────────────────────────────
    std::vector<Produs> lista;
    lista.reserve(depozit.produse().size());
    for (const auto &pereche : depozit.produse())
        lista.push_back(pereche.second);

    // ── 2. Numărăm tranzacțiile per produs (payload().id()) ──────────────────
    std::unordered_map<QString, int> nrTranz;
    for (const auto &t : istoric.toate())
        nrTranz[t.payload().id()]++;

    // ── 3. Sortăm conform opțiunii curente ───────────────────────────────────
    switch (m_sortProduse) {
    case SortProduse::PretAsc:
        std::sort(lista.begin(), lista.end(),
                  [](const Produs &a, const Produs &b){ return a.pret() < b.pret(); });
        break;
    case SortProduse::PretDesc:
        std::sort(lista.begin(), lista.end(),
                  [](const Produs &a, const Produs &b){ return a.pret() > b.pret(); });
        break;
    case SortProduse::CantiCresc:
        std::sort(lista.begin(), lista.end(),
                  [](const Produs &a, const Produs &b){ return a.cantitate() < b.cantitate(); });
        break;
    case SortProduse::CantiDesc:
        std::sort(lista.begin(), lista.end(),
                  [](const Produs &a, const Produs &b){ return a.cantitate() > b.cantitate(); });
        break;
    case SortProduse::NrTranzactii:
        std::sort(lista.begin(), lista.end(),
                  [&nrTranz](const Produs &a, const Produs &b){
                      return nrTranz[a.id()] > nrTranz[b.id()];
                  });
        break;
    default:
        break;
    }

    // ── 4. Populăm tabelul ───────────────────────────────────────────────────
    productsTable->setRowCount(0);

    for (const Produs &p : lista) {
        int row = productsTable->rowCount();
        productsTable->insertRow(row);

        auto *idItem = new QTableWidgetItem(p.id());
        idItem->setTextAlignment(Qt::AlignCenter);
        productsTable->setItem(row, 0, idItem);

        productsTable->setItem(row, 1, new QTableWidgetItem(p.nume()));

        auto *cantiItem = new QTableWidgetItem(QString::number(p.cantitate()));
        cantiItem->setTextAlignment(Qt::AlignCenter);
        productsTable->setItem(row, 2, cantiItem);

        productsTable->setItem(row, 3, new QTableWidgetItem(
            QString::number(p.pret(), 'f', 2)));

        int cnt = nrTranz.count(p.id()) ? nrTranz.at(p.id()) : 0;
        auto *tranzItem = new QTableWidgetItem(QString::number(cnt));
        tranzItem->setTextAlignment(Qt::AlignCenter);
        productsTable->setItem(row, 4, tranzItem);
    }

    // ── 5. Reaplică filtrul de căutare activ ─────────────────────────────────
    applyProductsSearch();
}

void MainWindow::setupAlertsPage(QWidget *page) {
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // --- 1. BARA DE SUS (Căutare și Acțiuni) ---
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->setSpacing(10);

    searchAlertsBar = new QLineEdit();
    searchAlertsBar->setPlaceholderText("Caută în alertele de stoc...");
    searchAlertsBar->setObjectName("SearchBar");
    searchAlertsBar->setMinimumHeight(35);
    searchAlertsBar->setClearButtonEnabled(true);

    // ComboBox de sortare alerte
    comboFilterAlerts = new QComboBox();
    comboFilterAlerts->setObjectName("BtnFilter");
    comboFilterAlerts->setMinimumHeight(35);
    comboFilterAlerts->setCursor(Qt::PointingHandCursor);
    comboFilterAlerts->addItem("▼ Sortare implicită");             // index 0 → Default
    comboFilterAlerts->addItem("Prag Alertă ↓ Descrescător");      // index 1 → PragDesc
    comboFilterAlerts->addItem("Prag Alertă ↑ Crescător");         // index 2 → PragCresc
    comboFilterAlerts->addItem("Cantitate ↑ Crescătoare");         // index 3 → CantiCresc
    comboFilterAlerts->addItem("Cantitate ↓ Descrescătoare");      // index 4 → CantiDesc
    comboFilterAlerts->addItem("Raport Cant./Prag ↑ (cel mai critic)"); // index 5 → Raport

    // Un buton specific pentru această pagină
    btnExportReport = new QPushButton("Export");
    btnExportReport->setObjectName("BtnFilter");
    btnExportReport->setMinimumHeight(35);
    btnExportReport->setCursor(Qt::PointingHandCursor);

    topBarLayout->addWidget(searchAlertsBar, 4);
    topBarLayout->addWidget(comboFilterAlerts, 2);
    topBarLayout->addWidget(btnExportReport, 1);

    mainLayout->addLayout(topBarLayout);

    // --- 2. TABELUL DE ALERTE ---
    alertsTable = new QTableWidget();
    alertsTable->setObjectName("MainProductsTable"); // Refolosim culorile tabelului de produse

    // Nu avem nevoie de coloana de preț aici, ne interesează Stoc vs Prag
    alertsTable->setColumnCount(4);
    alertsTable->setHorizontalHeaderLabels({"ID", "Nume Produs", "Stoc Curent", "Prag Alertă"});
    alertsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    alertsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    alertsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    alertsTable->verticalHeader()->setVisible(false);
    alertsTable->setShowGrid(false);
    alertsTable->setFrameShape(QFrame::NoFrame);

    mainLayout->addWidget(alertsTable);
    populateAlertsTable();

    // Double-click → detalii produs (col 0 = ID)
    connect(alertsTable, &QTableWidget::cellDoubleClicked,
            this, [this](int row, int) {
        if (auto *item = alertsTable->item(row, 0))
            showProductDetails(item->text());
    });

    // --- 3. CĂUTARE DINAMICĂ ---
    connect(searchAlertsBar, &QLineEdit::textChanged, this, [this]() { applyAlertsSearch(); });

    // --- 4. SORTARE ---
    connect(comboFilterAlerts, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        static const SortAlerte mapare[] = {
            SortAlerte::Default,
            SortAlerte::PragDesc,  SortAlerte::PragCresc,
            SortAlerte::CantiCresc, SortAlerte::CantiDesc,
            SortAlerte::Raport
        };
        m_sortAlerte = mapare[index];
        populateAlertsTable();
    });
}

void MainWindow::populateAlertsTable()
{
    // ── 1. Luăm produsele sub prag ────────────────────────────────────────────
    std::vector<Produs> listaAlerte = depozit.produseSubPrag();

    // ── 2. Sortăm conform opțiunii curente ───────────────────────────────────
    switch (m_sortAlerte) {
    case SortAlerte::PragDesc:
        std::sort(listaAlerte.begin(), listaAlerte.end(),
                  [](const Produs &a, const Produs &b){ return a.pragAlerta() > b.pragAlerta(); });
        break;
    case SortAlerte::PragCresc:
        std::sort(listaAlerte.begin(), listaAlerte.end(),
                  [](const Produs &a, const Produs &b){ return a.pragAlerta() < b.pragAlerta(); });
        break;
    case SortAlerte::CantiCresc:
        std::sort(listaAlerte.begin(), listaAlerte.end(),
                  [](const Produs &a, const Produs &b){ return a.cantitate() < b.cantitate(); });
        break;
    case SortAlerte::CantiDesc:
        std::sort(listaAlerte.begin(), listaAlerte.end(),
                  [](const Produs &a, const Produs &b){ return a.cantitate() > b.cantitate(); });
        break;
    case SortAlerte::Raport:
        // Raport cant/prag crescător = cel mai critic (aproape 0) primul
        // Dacă pragAlertă == 0 → tratăm ca ∞ (produs non-critic)
        std::sort(listaAlerte.begin(), listaAlerte.end(),
                  [](const Produs &a, const Produs &b) {
                      double ra = (a.pragAlerta() > 0) ? (double)a.cantitate() / a.pragAlerta() : 1e18;
                      double rb = (b.pragAlerta() > 0) ? (double)b.cantitate() / b.pragAlerta() : 1e18;
                      return ra < rb;
                  });
        break;
    default:
        break;
    }

    // ── 3. Populăm tabelul ───────────────────────────────────────────────────
    alertsTable->setRowCount(0);

    for (const Produs &prod : listaAlerte) {
        int row = alertsTable->rowCount();
        alertsTable->insertRow(row);

        alertsTable->setItem(row, 0, new QTableWidgetItem(prod.id()));
        alertsTable->setItem(row, 1, new QTableWidgetItem(prod.nume()));

        QTableWidgetItem *itemStoc = new QTableWidgetItem(QString::number(prod.cantitate()));
        itemStoc->setForeground(QBrush(QColor("#dc3545")));
        itemStoc->setFont(QFont("Arial", 10, QFont::Bold));
        itemStoc->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *itemPrag = new QTableWidgetItem(QString::number(prod.pragAlerta()));
        itemPrag->setTextAlignment(Qt::AlignCenter);

        alertsTable->setItem(row, 2, itemStoc);
        alertsTable->setItem(row, 3, itemPrag);
    }

    // ── 4. Reaplică filtrul de căutare activ ─────────────────────────────────
    applyAlertsSearch();
}

void MainWindow::setupHistoryPage(QWidget *page) {
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // --- 1. BARA DE SUS (Căutare și Filtre) ---
    QHBoxLayout *topBarLayout = new QHBoxLayout();

    searchHistoryBar = new QLineEdit();
    searchHistoryBar->setPlaceholderText("Caută după produs, companie sau ID tranzacție...");
    searchHistoryBar->setObjectName("SearchBar");
    searchHistoryBar->setMinimumHeight(35);
    searchHistoryBar->setClearButtonEnabled(true);

    btnExportHistory = new QPushButton("Exportă Istoric");
    btnExportHistory->setObjectName("BtnFilter"); // Stilul gri definit anterior
    btnExportHistory->setMinimumHeight(35);

    topBarLayout->addWidget(searchHistoryBar, 4);
    topBarLayout->addWidget(btnExportHistory, 1);
    mainLayout->addLayout(topBarLayout);

    // --- 2. TABELUL DE TRANZACȚII ---
    historyTable = new QTableWidget();
    historyTable->setObjectName("MainProductsTable");

    // Configurăm coloanele conform atributelor tale
    historyTable->setColumnCount(7);
    historyTable->setHorizontalHeaderLabels({
        "Data/Ora", "ID", "Produs", "Tip", "Companie", "Cantitate", "Total (RON)"
    });

    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->verticalHeader()->setVisible(false);
    historyTable->setShowGrid(false);

    mainLayout->addWidget(historyTable);

    // --- 3. LOGICA DE CĂUTARE DINAMICĂ ---
    connect(searchHistoryBar, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString t = text.toLower();
        for (int i = 0; i < historyTable->rowCount(); ++i) {
            bool match = false;
            // Căutăm în ID (Col 1), Produs (Col 2) și Companie (Col 4)
            if (historyTable->item(i, 1)->text().toLower().contains(t) ||
                historyTable->item(i, 2)->text().toLower().contains(t) ||
                historyTable->item(i, 4)->text().toLower().contains(t)) {
                match = true;
            }
            historyTable->setRowHidden(i, !match);
        }
    });

    populateHistoryTable();
}

void MainWindow::populateHistoryTable() {
    // Presupunem că ai o metodă în depozit care returnează vectorul de tranzacții
    auto tranzactii = istoric.toate();

    historyTable->setRowCount(0);

    for (const auto& t : tranzactii) {
        int row = historyTable->rowCount();
        historyTable->insertRow(row);

        // 1. Data și Ora (formatate frumos)
        QString dataStr = t.timestamp().toString("dd/MM/yyyy HH:mm");
        historyTable->setItem(row, 0, new QTableWidgetItem(dataStr));

        // 2. ID Tranzacție (folosim font monospace pentru aspect tehnic)
        QTableWidgetItem *idItem = new QTableWidgetItem(t.id());
        idItem->setFont(QFont("Monospace", 9));
        historyTable->setItem(row, 1, idItem);

        // 3. Produs
        historyTable->setItem(row, 2, new QTableWidgetItem(t.numeProdus()));

        // 4. Tip Tranzacție (cu Badge de culoare)
        QTableWidgetItem *tipItem = new QTableWidgetItem();
        if (t.tip() == TipTranzactie::Achizitionare) {
            tipItem->setText("➕ ACHIZIȚIE");
            tipItem->setForeground(QBrush(QColor("#198754"))); // Verde
        } else {
            tipItem->setText("➖ VÂNZARE");
            tipItem->setForeground(QBrush(QColor("#dc3545"))); // Roșu
        }
        tipItem->setFont(QFont("Arial", 9, QFont::Bold));
        historyTable->setItem(row, 3, tipItem);

        // 5. Companie
        historyTable->setItem(row, 4, new QTableWidgetItem(t.numeCompanie()));

        // 6. Cantitate
        historyTable->setItem(row, 5, new QTableWidgetItem(QString::number(t.cantitate())));

        // 7. Total (Cantitate * Pret Unitar)
        double total = t.cantitate() * t.pretUnitar();
        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(total, 'f', 2) + " RON");
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        historyTable->setItem(row, 6, totalItem);
    }
}

void MainWindow::onBtnSalesClicked()
{
    // 1. Verificăm că există cel puțin un produs în depozit
    if (depozit.esteGol()) {
        QMessageBox::warning(this,
                             "Niciun produs",
                             "Nu există produse în depozit.\n"
                             "Adaugă cel puțin un produs înainte de a înregistra o tranzacție.");
        return;
    }

    // 2. Deschidem dialogul — îi pasăm depozitul ca să populeze ComboBox-ul
    TransactionDialog dialog(depozit, this);

    if (dialog.exec() != QDialog::Accepted)
        return;   // utilizatorul a apăsat Anulează

    // 3. Extragem datele completate de utilizator
    TransactionData td = dialog.getTransactionData();

    // 4. Validare: produsul ales există în depozit?
    const Produs *produsPtr = depozit.gasesteProdusDupaId(td.produsId);
    if (!produsPtr) {
        QMessageBox::critical(this, "Eroare", "Produsul selectat nu mai există în depozit.");
        return;
    }
    const Produs produsSnapshot = *produsPtr;   // copie — payload pentru tranzacție

    // 5. Modificăm stocul în WarehouseManager (operatorii += / -=)
    try {
        if (td.tip == TipTranzactie::Achizitionare) {
            depozit.adaugaCantitate(td.produsId, td.cantitate);
        } else {
            depozit.scadeCantitate(td.produsId, td.cantitate);  // aruncă dacă stoc insuficient
        }
    } catch (const std::underflow_error &e) {
        QMessageBox::warning(this,
                             "Stoc insuficient",
                             QString("Nu se poate efectua vânzarea:\n%1").arg(e.what()));
        return;
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Eroare", e.what());
        return;
    }

    // 6. Construim obiectul Tranzactie<Produs> și îl adăugăm în istoric
    TranzactieProdus tranzactie(
        produsSnapshot.nume(),  // m_numeProdus
        td.tip,                 // m_tip
        td.companie,            // m_numeCompanie
        td.cantitate,           // m_cantitate
        td.pretUnitar,          // m_pretUnitar
        produsSnapshot          // m_payload — snapshot Produs
        );
    // m_id și m_timestamp sunt generate automat în constructorul Tranzactie

    istoric.adauga(tranzactie);   // adaugă în memorie + salvează automat prin storage

    // 7. Salvăm și stocul actualizat
    depozit.salveazaDate();

    // 8. Reîmprospătăm UI-ul
    UpdateUI();

    // 9. Navigăm la istoricul de tranzacții ca feedback vizual
    stackedWidget->setCurrentIndex(3);
}

void MainWindow::UpdateUI(){
    populateDashboard();
    populateProductsTable();
    populateAlertsTable();
    populateHistoryTable();
}

void MainWindow::applyProductsSearch()
{
    if (!searchBar || !productsTable) return;
    const QString text = searchBar->text().toLower();
    for (int row = 0; row < productsTable->rowCount(); ++row) {
        QTableWidgetItem *item = productsTable->item(row, 1); // coloana Nume
        bool match = text.isEmpty() || (item && item->text().toLower().contains(text));
        productsTable->setRowHidden(row, !match);
    }
}

void MainWindow::applyAlertsSearch()
{
    if (!searchAlertsBar || !alertsTable) return;
    const QString text = searchAlertsBar->text().toLower();
    for (int row = 0; row < alertsTable->rowCount(); ++row) {
        QTableWidgetItem *item = alertsTable->item(row, 1); // coloana Nume
        bool match = text.isEmpty() || (item && item->text().toLower().contains(text));
        alertsTable->setRowHidden(row, !match);
    }
}

void MainWindow::showProductDetails(const QString &produsId)
{
    const Produs *p = depozit.gasesteProdusDupaId(produsId);
    if (!p) return;

    // Colectăm tranzacțiile care aparțin acestui produs (după ID-ul din payload)
    std::vector<TranzactieProdus> tranzactiiProdus;
    for (const auto &t : istoric.toate())
        if (t.payload().id() == produsId)
            tranzactiiProdus.push_back(t);

    ProductDetailsDialog dialog(*p, tranzactiiProdus, this);
    dialog.exec();
}

MainWindow::~MainWindow()
{
    delete ui;
}
