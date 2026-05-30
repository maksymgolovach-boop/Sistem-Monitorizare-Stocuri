#ifndef ADDPRODUCTDIALOG_H
#define ADDPRODUCTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QUuid>

// O structură simplă pentru a transporta datele din dialog în fereastra principală
struct ProductData {
    QString ID;
    QString nume;
    int cantitate;
    double pret;
    int PragAlerta;
};

class AddProductDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddProductDialog(QWidget *parent = nullptr);

    // Funcție prin care MainWindow va „extrage” datele după ce userul apasă Salvează
    ProductData getProductData() const;

private:
    QString generatedId;

    QLineEdit *editNume;
    QSpinBox *spinCantitate;
    QDoubleSpinBox *spinPret;
    QSpinBox *spinPrag;

    QPushButton *btnSave;
    QPushButton *btnCancel;

    void setupUI();
};
#endif // ADDPRODUCTDIALOG_H
