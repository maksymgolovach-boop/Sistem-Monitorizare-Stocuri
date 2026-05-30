#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../ManagerStocare/jsonstorage.h"
#include "../NivelStocareDate/Tranzactiemanager.h"
#include "../NivelStocareDate/warehousemanager.h"
#include <QPushButton>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QPushButton* createMenuBtn(QString text, bool active);

private:
    Ui::MainWindow *ui;
    JsonStorage m_storagedepozit;
    JsonStorage m_storagetranzactii;

    WarehouseManager depozit;
    IstoricTranzactii istoric;

    // ── Enumerații sortare ────────────────────────────────────────────────────
    enum class SortProduse { Default, PretAsc, PretDesc, CantiCresc, CantiDesc, NrTranzactii };
    enum class SortAlerte  { Default, PragDesc, PragCresc, CantiCresc, CantiDesc, Raport };

    void setupLayout();
    void setupHeader();
    void setupStatusBar();
    void setupSidebar();
    void setupDashboardPage(QWidget *page);
    void populateDashboard();

    void setupProductsPage(QWidget *page);
    void populateProductsTable();

    void setupAlertsPage(QWidget *page);
    void populateAlertsTable();

    void setupHistoryPage(QWidget *page);
    void populateHistoryTable();

    void onBtnSalesClicked();
    void UpdateUI();

    /** Aplică simultan filtrul text + filtrul dată pe historyTable. */
    void applyHistoryFilter();

    /** Generează factura PDF pentru rândul selectat din historyTable. */
    void generateInvoice(int row);

    void applyProductsSearch();
    void applyAlertsSearch();

    void updateHeaderBadges();   // sincronizează badge-urile din header cu datele live
    void updateClock();          // actualizează ceasul din status bar

    /**
     * @brief Marchează butonul @p active ca selectat în sidebar și resetează restul.
     *
     * Apelează unpolish/polish pe fiecare buton nav pentru a forța Qt să
     * re-evalueze regulile QSS cu proprietatea dinamică "active" actualizată.
     */
    void activateSidebarBtn(QPushButton *active);

    /** Deschide ProductDetailsDialog pentru produsul cu ID-ul dat. */
    void showProductDetails(const QString &produsId);

    // Zonele principale
    QWidget *sidebar;
    QWidget *header;
    QWidget *mainPanel;

    // Header — badge-uri live
    QLabel  *m_headerProduseBadge = nullptr;
    QLabel  *m_headerAlertBadge   = nullptr;
    QLabel  *m_headerTransBadge   = nullptr;

    // Status bar — ceas live
    QLabel  *m_statusClockLabel   = nullptr;
    QTimer  *m_clockTimer         = nullptr;

    QWidget* createStatCard(QString title, QString value, QString objectName);

    // Butoanele din Sidebar
    QPushButton *btnDashboard, *btnProducts, *btnAlerts;
    QPushButton *btnSales, *btnPurchases, *btnHistory;

    // "Inima" panoului principal
    QStackedWidget *stackedWidget;

    // Widget-uri pentru pagina de Produse
    QLineEdit *searchBar;
    QPushButton *btnFilter;
    QPushButton *btnEdit;
    QPushButton *btnAddProduct;
    QPushButton *btnDeleteProduct;
    QTableWidget *productsTable;

    // ── Stare sortare ─────────────────────────────────────────────────────────
    SortProduse  m_sortProduse       = SortProduse::Default;
    SortAlerte   m_sortAlerte        = SortAlerte::Default;

    QComboBox   *comboFilterProducts = nullptr;
    QComboBox   *comboFilterAlerts   = nullptr;

    // Elemente UI pentru Dashboard
    QLabel       *dashboardAlertValue  = nullptr;  // valoarea din cardul "Sub Prag Alertă"
    QLabel       *dashboardTransValue  = nullptr;  // valoarea din cardul "Tranzacții"
    QTableWidget *dashboardTable       = nullptr;  // tabelul de produse din dashboard

    // Elemente UI pentru pagina de Alerte
    QLineEdit *searchAlertsBar;
    QTableWidget *alertsTable;

    // Elemente UI pentru pagina de istoric tranzactii
    QTableWidget *historyTable        = nullptr;
    QLineEdit    *searchHistoryBar    = nullptr;
    QPushButton  *btnExportHistory    = nullptr;
    QPushButton  *btnFactura          = nullptr;
    QDateEdit    *dateFilterFrom      = nullptr;
    QDateEdit    *dateFilterTo        = nullptr;
    QPushButton  *btnResetDate        = nullptr;
};
#endif // MAINWINDOW_H
