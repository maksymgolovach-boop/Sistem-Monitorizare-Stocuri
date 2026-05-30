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

    void setupLayout(); // Metoda unde construim structura
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

    void applyProductsSearch();
    void applyAlertsSearch();

    // Zonele principale
    QWidget *sidebar;
    QWidget *header;
    QWidget *mainPanel;

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
    QPushButton *btnExportReport;
    QTableWidget *alertsTable;

    // Elemente UI pentru pagina de istoric tranzactii
    QTableWidget *historyTable;
    QLineEdit *searchHistoryBar;
    QPushButton *btnExportHistory;
};
#endif // MAINWINDOW_H
