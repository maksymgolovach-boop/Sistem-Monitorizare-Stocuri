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
#include <QMenu>
#include <QFileDialog>
#include <QDateTime>
#include "exportmanager.h"
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
    header->setFixedHeight(56);
    header->setObjectName("Header");
    setupHeader();
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

    // Status bar + valori inițiale
    setupStatusBar();
    updateHeaderBadges();
}

void MainWindow::setupHeader()
{
    QHBoxLayout *lay = new QHBoxLayout(header);
    lay->setContentsMargins(16, 0, 16, 0);
    lay->setSpacing(0);

    // ── STÂNGA: Logo + titlu + separator + nume depozit ───────────────────────
    QLabel *lblLogo = new QLabel("📦");
    lblLogo->setStyleSheet("font-size: 22px; background: transparent;");

    QLabel *lblTitle = new QLabel("StocManager");
    lblTitle->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #212529;"
        "background: transparent; margin-left: 8px;");

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color: #dee2e6; margin: 12px 14px;");

    QSettings settings;
    QString depotName = settings.value("depot/name", "Depozit Central").toString();
    QLabel *lblDepot = new QLabel(depotName);
    lblDepot->setStyleSheet(
        "font-size: 12px; color: #6c757d; background: transparent;");

    lay->addWidget(lblLogo);
    lay->addWidget(lblTitle);
    lay->addWidget(sep);
    lay->addWidget(lblDepot);
    lay->addStretch();

    // ── DREAPTA: Badge-uri status ─────────────────────────────────────────────
    auto makeBadge = [](const QString &text,
                        const QString &bg, const QString &fg,
                        const QString &border) -> QLabel * {
        QLabel *b = new QLabel(text);
        b->setStyleSheet(
            QString("background:%1; color:%2; border:1px solid %3;"
                    "border-radius:10px; padding:2px 10px;"
                    "font-size:11px; font-weight:bold;")
                .arg(bg, fg, border));
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        return b;
    };

    m_headerProduseBadge = makeBadge("📦  0 produse",
                                     "#e7f1ff", "#0d6efd", "#b6d4fe");
    m_headerAlertBadge   = makeBadge("✓  0 alerte",
                                     "#e6f4ea", "#198754", "#b7e4c7");
    m_headerTransBadge   = makeBadge("🔄  0 tranzacții",
                                     "#f3e8ff", "#6f42c1", "#d8b4fe");

    lay->addWidget(m_headerProduseBadge);
    lay->addSpacing(8);
    lay->addWidget(m_headerAlertBadge);
    lay->addSpacing(8);
    lay->addWidget(m_headerTransBadge);
}

void MainWindow::setupStatusBar()
{
    // Stânga: informație aplicație
    QLabel *lblInfo = new QLabel("  Sistem Monitorizare Stocuri  v0.1");
    lblInfo->setObjectName("StatusBarLabel");
    statusBar()->addWidget(lblInfo);

    // Dreapta: dată + ceas live (actualizat la fiecare minut)
    m_statusClockLabel = new QLabel();
    m_statusClockLabel->setObjectName("StatusBarLabel");
    m_statusClockLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusBar()->addPermanentWidget(m_statusClockLabel);

    // Afișare imediată + timer la 60 secunde
    updateClock();
    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(60000);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start();
}

void MainWindow::updateClock()
{
    if (!m_statusClockLabel) return;
    m_statusClockLabel->setText(
        QDateTime::currentDateTime().toString("  📅  dd/MM/yyyy    🕒  HH:mm  "));
}

void MainWindow::updateHeaderBadges()
{
    if (!m_headerProduseBadge || !m_headerAlertBadge || !m_headerTransBadge)
        return;

    const int nrProduse = depozit.numarProduse();
    const int nrAlerte  = static_cast<int>(depozit.produseSubPrag().size());
    const int nrTranz   = istoric.numar();

    m_headerProduseBadge->setText(
        QString("📦  %1 produs%2")
            .arg(nrProduse)
            .arg(nrProduse == 1 ? "" : "e"));

    m_headerTransBadge->setText(
        QString("🔄  %1 tranzacți%2")
            .arg(nrTranz)
            .arg(nrTranz == 1 ? "e" : "i"));

    // Alert badge: verde dacă totul OK, roșu dacă există alerte
    m_headerAlertBadge->setText(
        nrAlerte > 0
            ? QString("⚠  %1 alertă%2").arg(nrAlerte).arg(nrAlerte == 1 ? "" : "e")
            : QString("✓  Stoc OK"));

    const QString alertStyle = nrAlerte > 0
        ? "background:#fff5f5; color:#dc3545; border:1px solid #ffc9c9;"
          "border-radius:10px; padding:2px 10px; font-size:11px; font-weight:bold;"
        : "background:#e6f4ea; color:#198754; border:1px solid #b7e4c7;"
          "border-radius:10px; padding:2px 10px; font-size:11px; font-weight:bold;";
    m_headerAlertBadge->setStyleSheet(alertStyle);
}

void MainWindow::activateSidebarBtn(QPushButton *active)
{
    const QList<QPushButton*> navBtns = {btnDashboard, btnProducts, btnAlerts, btnHistory};
    for (QPushButton *btn : navBtns) {
        btn->setProperty("active", btn == active);
        // unpolish + polish forțează Qt să re-evalueze regulile QSS
        // cu noua valoare a proprietății dinamice "active"
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
        btn->update();
    }
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

    btnDashboard->setProperty("active", true);
    btnProducts->setProperty("active", false);
    btnAlerts->setProperty("active", false);

    sideLayout->addWidget(btnDashboard);
    sideLayout->addWidget(btnProducts);
    sideLayout->addWidget(btnAlerts);

    sideLayout->addSpacing(20); // Spațiu între secțiuni

    // --- SECTIUNEA TRANZACTII ---
    QLabel *lblTranzactii = new QLabel("TRANZACTII");
    sideLayout->addWidget(lblTranzactii);

    btnSales = new QPushButton("Tranzactie noua");
    btnHistory = new QPushButton("Istoric Tranzactii");
    btnHistory->setProperty("active", false);

    sideLayout->addWidget(btnSales);
    sideLayout->addWidget(btnHistory);

    connect(btnDashboard, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(0);
        activateSidebarBtn(btnDashboard);
    });
    connect(btnProducts, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(1);
        activateSidebarBtn(btnProducts);
    });
    connect(btnAlerts, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(2);
        activateSidebarBtn(btnAlerts);
    });
    connect(btnHistory, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(3);
        activateSidebarBtn(btnHistory);
    });

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
    dashboardTable->setColumnCount(5);
    dashboardTable->setHorizontalHeaderLabels(
        {"Produs", "Categorie", "Preț (RON)", "Stoc", "Status"});
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

        dashboardTable->setItem(row, 1, new QTableWidgetItem(p->categorie())); // Categorie

        dashboardTable->setItem(row, 2, new QTableWidgetItem(
            QString::number(p->pret(), 'f', 2)));
        dashboardTable->setItem(row, 3, new QTableWidgetItem(
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
        dashboardTable->setItem(row, 4, statusItem);
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

    productsTable->setColumnCount(6);
    productsTable->setHorizontalHeaderLabels(
        {"ID", "Nume Produs", "Categorie", "Stoc", "Preț (RON)", "Nr. Tranzacții"});
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

            Produs nouprodus(dateNoi.nume, dateNoi.pret, dateNoi.cantitate,
                             dateNoi.PragAlerta, dateNoi.categorie);
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

        productsTable->setItem(row, 2, new QTableWidgetItem(p.categorie()));  // Categorie

        auto *cantiItem = new QTableWidgetItem(QString::number(p.cantitate()));
        cantiItem->setTextAlignment(Qt::AlignCenter);
        productsTable->setItem(row, 3, cantiItem);

        productsTable->setItem(row, 4, new QTableWidgetItem(
            QString::number(p.pret(), 'f', 2)));

        int cnt = nrTranz.count(p.id()) ? nrTranz.at(p.id()) : 0;
        auto *tranzItem = new QTableWidgetItem(QString::number(cnt));
        tranzItem->setTextAlignment(Qt::AlignCenter);
        productsTable->setItem(row, 5, tranzItem);
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

    // --- 5. EXPORT ---
    connect(btnExportReport, &QPushButton::clicked, this, [this]() {
        QMenu menu(this);
        QAction *csvAct = menu.addAction("📄  Export CSV");
        QAction *pdfAct = menu.addAction("📋  Export PDF");
        QAction *chosen = menu.exec(
            btnExportReport->mapToGlobal(QPoint(0, btnExportReport->height())));

        if (chosen == csvAct) {
            QString file = QFileDialog::getSaveFileName(
                this, "Salvează CSV", "alerte_stoc.csv", "CSV (*.csv)");
            if (file.isEmpty()) return;
            if (ExportManager::exportCSV(file, alertsTable))
                QMessageBox::information(this, "Export reușit",
                    "Fișierul CSV a fost salvat cu succes:\n" + file);
            else
                QMessageBox::critical(this, "Eroare export",
                    "Nu s-a putut scrie fișierul CSV.");

        } else if (chosen == pdfAct) {
            QString file = QFileDialog::getSaveFileName(
                this, "Salvează PDF", "alerte_stoc.pdf", "PDF (*.pdf)");
            if (file.isEmpty()) return;
            const int n = static_cast<int>(depozit.produseSubPrag().size());
            const QStringList info = {
                QString("Total produse sub prag de alertă: %1").arg(n),
                QString("Raport generat la: %1")
                    .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"))
            };
            if (ExportManager::exportPDF(file, alertsTable, "Raport Alerte Stoc", info))
                QMessageBox::information(this, "Export reușit",
                    "Raportul PDF a fost salvat cu succes:\n" + file);
            else
                QMessageBox::critical(this, "Eroare export",
                    "Nu s-a putut genera fișierul PDF.");
        }
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

    // --- 1. BARA DE SUS: Căutare + Factură + Export ---
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->setSpacing(10);

    searchHistoryBar = new QLineEdit();
    searchHistoryBar->setPlaceholderText("Caută după produs, companie sau ID tranzacție...");
    searchHistoryBar->setObjectName("SearchBar");
    searchHistoryBar->setMinimumHeight(35);
    searchHistoryBar->setClearButtonEnabled(true);

    btnFactura = new QPushButton("🖨  Scoate Factură");
    btnFactura->setObjectName("BtnDialogSave");
    btnFactura->setMinimumHeight(35);
    btnFactura->setCursor(Qt::PointingHandCursor);
    btnFactura->setEnabled(false);   // activ doar când e selectat un rând

    btnExportHistory = new QPushButton("Exportă Istoric");
    btnExportHistory->setObjectName("BtnFilter");
    btnExportHistory->setMinimumHeight(35);

    topBarLayout->addWidget(searchHistoryBar, 4);
    topBarLayout->addWidget(btnFactura, 1);
    topBarLayout->addWidget(btnExportHistory, 1);
    mainLayout->addLayout(topBarLayout);

    // --- 2. BARA DE FILTRARE DATĂ ---
    QHBoxLayout *dateBarLayout = new QHBoxLayout();
    dateBarLayout->setSpacing(8);

    QLabel *lblDate = new QLabel("📅  Interval:");
    lblDate->setStyleSheet("color: #495057; font-weight: bold;");

    dateFilterFrom = new QDateEdit(QDate(2000, 1, 1));
    dateFilterFrom->setCalendarPopup(true);
    dateFilterFrom->setDisplayFormat("dd/MM/yyyy");
    dateFilterFrom->setMinimumHeight(32);
    dateFilterFrom->setMinimumWidth(128);
    dateFilterFrom->setObjectName("ModernInput");

    QLabel *lblArrow = new QLabel("→");
    lblArrow->setStyleSheet("color: #6c757d;");

    dateFilterTo = new QDateEdit(QDate::currentDate());
    dateFilterTo->setCalendarPopup(true);
    dateFilterTo->setDisplayFormat("dd/MM/yyyy");
    dateFilterTo->setMinimumHeight(32);
    dateFilterTo->setMinimumWidth(128);
    dateFilterTo->setObjectName("ModernInput");

    btnResetDate = new QPushButton("✕ Resetează");
    btnResetDate->setObjectName("BtnFilter");
    btnResetDate->setMinimumHeight(32);
    btnResetDate->setCursor(Qt::PointingHandCursor);

    dateBarLayout->addWidget(lblDate);
    dateBarLayout->addWidget(dateFilterFrom);
    dateBarLayout->addWidget(lblArrow);
    dateBarLayout->addWidget(dateFilterTo);
    dateBarLayout->addWidget(btnResetDate);
    dateBarLayout->addStretch();
    mainLayout->addLayout(dateBarLayout);

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

    // --- 3. CĂUTARE + FILTRU DATĂ ---
    connect(searchHistoryBar, &QLineEdit::textChanged,
            this, [this]() { applyHistoryFilter(); });
    connect(dateFilterFrom, &QDateEdit::dateChanged,
            this, [this]() { applyHistoryFilter(); });
    connect(dateFilterTo, &QDateEdit::dateChanged,
            this, [this]() { applyHistoryFilter(); });
    connect(btnResetDate, &QPushButton::clicked, this, [this]() {
        dateFilterFrom->setDate(QDate(2000, 1, 1));
        dateFilterTo->setDate(QDate::currentDate());
        applyHistoryFilter();
    });

    // Activează butonul de factură doar când e selectat un rând
    connect(historyTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        btnFactura->setEnabled(historyTable->selectionModel()->hasSelection());
    });

    // Generare factură
    connect(btnFactura, &QPushButton::clicked, this, [this]() {
        int row = historyTable->currentRow();
        if (row >= 0) generateInvoice(row);
    });

    populateHistoryTable();

    // --- 4. EXPORT ---
    connect(btnExportHistory, &QPushButton::clicked, this, [this]() {
        QMenu menu(this);
        QAction *csvAct = menu.addAction("📄  Export CSV");
        QAction *pdfAct = menu.addAction("📋  Export PDF");
        QAction *chosen = menu.exec(
            btnExportHistory->mapToGlobal(QPoint(0, btnExportHistory->height())));

        if (chosen == csvAct) {
            QString file = QFileDialog::getSaveFileName(
                this, "Salvează CSV", "istoric_tranzactii.csv", "CSV (*.csv)");
            if (file.isEmpty()) return;
            if (ExportManager::exportCSV(file, historyTable))
                QMessageBox::information(this, "Export reușit",
                    "Fișierul CSV a fost salvat cu succes:\n" + file);
            else
                QMessageBox::critical(this, "Eroare export",
                    "Nu s-a putut scrie fișierul CSV.");

        } else if (chosen == pdfAct) {
            QString file = QFileDialog::getSaveFileName(
                this, "Salvează PDF", "istoric_tranzactii.pdf", "PDF (*.pdf)");
            if (file.isEmpty()) return;

            // Calculăm statisticile sumar pentru header-ul PDF-ului
            double valAchiz = 0.0, valVanz = 0.0;
            int nrAchiz = 0, nrVanz = 0;
            for (const auto &t : istoric.toate()) {
                if (t.tip() == TipTranzactie::Achizitionare) {
                    valAchiz += t.valoareTotala(); ++nrAchiz;
                } else {
                    valVanz  += t.valoareTotala(); ++nrVanz;
                }
            }
            const QStringList info = {
                QString("Total tranzacții: %1  |  Achiziții: %2  |  Vânzări: %3")
                    .arg(istoric.numar()).arg(nrAchiz).arg(nrVanz),
                QString("Valoare totală achiziții: %1 RON  |  Valoare totală vânzări: %2 RON")
                    .arg(valAchiz, 0, 'f', 2).arg(valVanz, 0, 'f', 2)
            };
            if (ExportManager::exportPDF(file, historyTable, "Raport Istoric Tranzacții", info))
                QMessageBox::information(this, "Export reușit",
                    "Raportul PDF a fost salvat cu succes:\n" + file);
            else
                QMessageBox::critical(this, "Eroare export",
                    "Nu s-a putut genera fișierul PDF.");
        }
    });
}

void MainWindow::populateHistoryTable() {
    // Presupunem că ai o metodă în depozit care returnează vectorul de tranzacții
    auto tranzactii = istoric.toate();

    historyTable->setRowCount(0);

    for (const auto& t : tranzactii) {
        int row = historyTable->rowCount();
        historyTable->insertRow(row);

        // 1. Data și Ora (formatate frumos); QDateTime stocat în UserRole pentru filtrul de dată
        QString dataStr = t.timestamp().toString("dd/MM/yyyy HH:mm");
        QTableWidgetItem *dateItem = new QTableWidgetItem(dataStr);
        dateItem->setData(Qt::UserRole, t.timestamp());
        historyTable->setItem(row, 0, dateItem);

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
    updateHeaderBadges();
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

void MainWindow::applyHistoryFilter()
{
    if (!searchHistoryBar || !historyTable || !dateFilterFrom || !dateFilterTo)
        return;

    const QString text  = searchHistoryBar->text().toLower();
    const QDate   from  = dateFilterFrom->date();
    const QDate   to    = dateFilterTo->date();

    for (int i = 0; i < historyTable->rowCount(); ++i) {
        // ── Filtru text: ID (col 1), Produs (col 2), Companie (col 4) ──────────
        bool textOk = text.isEmpty();
        if (!textOk) {
            auto cell = [&](int col) -> QString {
                auto *it = historyTable->item(i, col);
                return it ? it->text().toLower() : QString();
            };
            textOk = cell(1).contains(text) ||
                     cell(2).contains(text) ||
                     cell(4).contains(text);
        }

        // ── Filtru dată: compară QDate din UserRole al col 0 ──────────────────
        bool dateOk = true;
        if (auto *it = historyTable->item(i, 0)) {
            QDate rowDate = it->data(Qt::UserRole).toDateTime().date();
            if (rowDate.isValid())
                dateOk = (rowDate >= from && rowDate <= to);
        }

        historyTable->setRowHidden(i, !(textOk && dateOk));
    }
}

void MainWindow::generateInvoice(int row)
{
    auto *idItem = historyTable->item(row, 1);
    if (!idItem) return;
    const QString tranzId = idItem->text();

    // Găsim tranzacția în istoric după ID
    const TranzactieProdus *tPtr = nullptr;
    for (const auto &t : istoric.toate())
        if (t.id() == tranzId) { tPtr = &t; break; }
    if (!tPtr) return;
    const TranzactieProdus &t = *tPtr;

    const QString defaultName =
        QString("factura_%1.pdf").arg(t.id().left(8).toUpper());
    QString file = QFileDialog::getSaveFileName(
        this, "Salvează Factură PDF", defaultName, "PDF (*.pdf)");
    if (file.isEmpty()) return;

    // ── Construim HTML-ul facturii ────────────────────────────────────────────
    const bool     isAchiz    = (t.tip() == TipTranzactie::Achizitionare);
    const QString  tipColor   = isAchiz ? "#198754" : "#dc3545";
    const QString  tipLabel   = isAchiz ? "ACHIZIȚIE" : "VÂNZARE";
    const QString  partyLabel = isAchiz ? "Furnizor" : "Client";
    const QString  invNumber  = t.id().left(8).toUpper();
    const QString  dataStr    = t.timestamp().toString("dd/MM/yyyy  HH:mm");

    const QString html = QString(R"(
<!DOCTYPE html><html><head><meta charset="UTF-8">
<style>
  body { font-family: Arial, Helvetica, sans-serif; font-size: 10pt; color: #212529; }
  .header-table { width: 100%%; border-bottom: 3px solid #212529;
                  margin-bottom: 16px; padding-bottom: 12px; }
  .co-name  { font-size: 17pt; font-weight: bold; }
  .co-sub   { font-size: 8.5pt; color: #6c757d; margin-top: 3px; }
  .inv-type { font-size: 15pt; font-weight: bold; color: %1; }
  .inv-meta { font-size: 9pt; color: #6c757d; margin-top: 3px; }
  .parties  { width: 100%%; margin-bottom: 18px; }
  .lbl      { font-size: 7.5pt; color: #6c757d; text-transform: uppercase;
               letter-spacing: 0.5px; margin-bottom: 3px; }
  .val      { font-size: 12pt; font-weight: bold; }
  table.items { border-collapse: collapse; width: 100%%; margin: 12px 0; }
  table.items thead tr { background-color: #212529; color: white; }
  table.items th { padding: 7px 10px; text-align: left; font-size: 9pt; }
  table.items td { padding: 7px 10px; border-bottom: 1px solid #dee2e6; }
  .total-block { text-align: right; margin-top: 8px; }
  .total-val   { font-size: 14pt; font-weight: bold; color: %1; }
  .footer { margin-top: 30px; border-top: 1px solid #dee2e6; padding-top: 8px;
             font-size: 7.5pt; color: #adb5bd; }
  .mono   { font-family: monospace; }
</style>
</head>
<body>
<table class="header-table" cellpadding="0" cellspacing="0">
  <tr>
    <td>
      <div class="co-name">Sistem Monitorizare Stocuri</div>
      <div class="co-sub">Software de gestiune depozit</div>
    </td>
    <td align="right">
      <div class="inv-type">FACTURĂ %2</div>
      <div class="inv-meta">Nr.&nbsp;%3</div>
      <div class="inv-meta">%4</div>
    </td>
  </tr>
</table>

<table class="parties" cellpadding="0" cellspacing="0">
  <tr>
    <td width="50%%">
      <div class="lbl">%5</div>
      <div class="val">%6</div>
    </td>
    <td width="50%%">
      <div class="lbl">Produs</div>
      <div class="val">%7</div>
    </td>
  </tr>
</table>

<table class="items">
  <thead>
    <tr>
      <th>Descriere produs</th>
      <th>Cantitate</th>
      <th>Preț unitar (RON)</th>
      <th>Valoare totală (RON)</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>%7</td>
      <td align="center">%8 buc.</td>
      <td align="right">%9</td>
      <td align="right">%10</td>
    </tr>
  </tbody>
</table>

<div class="total-block">
  <span class="total-val">TOTAL DE PLATĂ:&nbsp;&nbsp;%10 RON</span>
</div>

<div class="footer">
  Document generat automat &mdash; Sistem Monitorizare Stocuri<br/>
  <span class="mono">ID Tranzacție: %11</span>
</div>
</body></html>
)")
        .arg(tipColor)                                      // %1
        .arg(tipLabel)                                      // %2
        .arg(invNumber)                                     // %3
        .arg(dataStr)                                       // %4
        .arg(partyLabel)                                    // %5
        .arg(t.numeCompanie().toHtmlEscaped())              // %6
        .arg(t.numeProdus().toHtmlEscaped())                // %7
        .arg(t.cantitate())                                 // %8
        .arg(QString::number(t.pretUnitar(),    'f', 2))   // %9
        .arg(QString::number(t.valoareTotala(), 'f', 2))   // %10
        .arg(t.id());                                       // %11

    if (ExportManager::exportHTMLtoPDF(file, html, /*landscape=*/false))
        QMessageBox::information(this, "Factură generată",
            "Factura PDF a fost salvată cu succes:\n" + file);
    else
        QMessageBox::critical(this, "Eroare",
            "Nu s-a putut genera factura PDF.");
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
