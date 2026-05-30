#ifndef JSONSTORAGE_H
#define JSONSTORAGE_H

#include "IStocare.h"

class JsonStorage : public IStocare
{

public:
    /**
     * @brief Constructor implicit — fișierul nu este setat.
     *        Apelați setCaleFisier() sau alegeti fisier() înainte de a salva/încărca.
     */
    JsonStorage() = default;

    /**
     * @brief Constructor cu cale explicită — util când calea este deja cunoscută
     *        (ex: ultima sesiune salvată în QSettings).
     * @param caleFisier  Calea completă către fișierul JSON (ex: "/home/user/depozit.json").
     */
    explicit JsonStorage(const QString &caleFisier);

    ~JsonStorage() override = default;

    // ── Selecție fișier ───────────────────────────────────────────────────────

    bool alegeFisier(QWidget *parent = nullptr, bool modSalvare = false);

    /**
     * @brief Setează direct calea fișierului (fără dialog).
     *        Util când calea vine din QSettings sau din argumente CLI.
     */
    void setCaleFisier(const QString &caleFisier);

    /** @return Calea curentă sau QString() dacă nu a fost setată. */
    QString caleFisier() const;

    /** @return true dacă o cale a fost setată (dialog confirmat sau set manual). */
    bool areCaleFisier() const;

    // ── IStorage ──────────────────────────────────────────────────────────────
    void salveaza(const std::unordered_map<QString, Produs> &produse) override;
    std::unordered_map<QString, Produs> incarca()                     override;
    bool exista() const                                               override;

    void salveazaTranzactii(const std::vector<TranzactieProdus> &tranzactii) override;
    std::vector<TranzactieProdus> incarcaTranzactii()                        override;

private:
    QString m_caleFisier;

    /** Aruncă excepție dacă m_caleFisier este gol. */
    void verificaCale() const;

    /** Creează directorul parinte dacă nu există. */
    //void asiguraDirector() const;
    void asiguraDirector() const;
};

#endif // JSONSTORAGE_H
