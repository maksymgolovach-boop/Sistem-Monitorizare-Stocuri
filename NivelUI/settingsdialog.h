#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

/** Valorile editabile în fereastra de Setări. */
struct SettingsData {
    QString numeDepozit;      ///< Afișat în header sub logo
    QString caleDepozit;      ///< Cale completă către depozit.json
    QString caleTranzactii;   ///< Cale completă către tranzactii.json
};

/**
 * @brief Dialog modal pentru configurarea aplicației.
 *
 * Primește valorile curente prin constructor și le returnează
 * (după editare) prin getSettings() la Accepted.
 *
 * Secțiuni:
 *  - Identitate : numele depozitului afișat în header
 *  - Fișiere de date : căile JSON pentru produse și tranzacții,
 *    cu butoane Browse care deschid un dialog de selectare fișier
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const SettingsData &current, QWidget *parent = nullptr);

    /** Returnează datele completate de utilizator (valabil doar la Accepted). */
    SettingsData getSettings() const;

private:
    QLineEdit   *m_editNume;
    QLineEdit   *m_editCaleDepozit;
    QLineEdit   *m_editCaleTranzactii;
    QPushButton *m_btnBrowseDepozit;
    QPushButton *m_btnBrowseTranzactii;
    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;

    void setupUI(const SettingsData &current);
    bool valideaza();
};

#endif // SETTINGSDIALOG_H
