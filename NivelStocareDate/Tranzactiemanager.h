#ifndef TRANZACTIEMANAGER_H
#define TRANZACTIEMANAGER_H

#include "../LibrarieModele/Tranzactie.h"
#include "../ManagerStocare/IStocare.h"

#include <QString>
#include <QDateTime>

#include <stdexcept>
#include <vector>

/**
 * @brief Container în memorie pentru istoricul tranzacțiilor.
 *
 * TranzactieStorage nu știe nimic despre fișiere sau JSON.
 * Toată persistența este delegată către IStocare prin metodele
 * salveaza() și incarca(), care apelează explicit storage-ul injectat.
 *
 * Utilizare tipică:
 * @code
 *   JsonStorage storage("depozit.json");
 *   TranzactieStorage<Produs> istoric(&storage);
 *   istoric.incarca();                       // citește din fișier prin storage
 *   istoric.adauga(t);                       // adaugă în memorie
 *   istoric.salveaza();                      // scrie în fișier prin storage
 * @endcode
 */
template <typename T>
class TranzactieStorage
{
public:
    // ── Constructori ─────────────────────────────────────────────────────────

    /**
     *  Constructor cu storage injectat.
     *  storage  Pointer la implementarea de stocare (nu poate fi nullptr).
     *                 TranzactieStorage nu preia ownership-ul pointerului.
     */
    explicit TranzactieStorage(IStocare *storage)
        : m_storage(storage)
    {
        if (!m_storage)
            throw std::invalid_argument("TranzactieStorage: storage-ul nu poate fi nullptr.");
    }

    ~TranzactieStorage() = default;

    TranzactieStorage(const TranzactieStorage &)            = delete;
    TranzactieStorage &operator=(const TranzactieStorage &) = delete;

    // ── Persistență ───────────────────────────────────────────────────────────

    /**
     *  Încarcă istoricul din storage în memorie.
     *        Suprascrie orice date nesalvate din sesiunea curentă.
     * throws std::runtime_error dacă storage-ul raportează eroare de I/O.
     */
    void incarcaDate()
    {
        m_istoric = m_storage->incarcaTranzactii();
    }

    /**
     *  Salvează întreg istoricul din memorie prin storage.
     * throws std::runtime_error dacă storage-ul raportează eroare de I/O.
     */
    void salveaza()
    {
        m_storage->salveazaTranzactii(m_istoric);
    }

    // ── Operații pe istoric ───────────────────────────────────────────────────

    /**
     * @brief Adaugă o tranzacție în memorie și persistă automat prin storage.
     * @throws std::runtime_error dacă storage-ul raportează eroare de I/O.
     */
    void adauga(const Tranzactie<T> &tranzactie)
    {
        m_istoric.push_back(tranzactie);
        salveaza();
    }

    // ── Acces ─────────────────────────────────────────────────────────────────

    /** return Toate tranzacțiile din memorie. */
    const std::vector<Tranzactie<T>> &toate() const { return m_istoric; }

    /** return Numărul de tranzacții din istoric. */
    int numar() const { return static_cast<int>(m_istoric.size()); }

    /** return true dacă istoricul este gol. */
    bool esteGol() const { return m_istoric.empty(); }

    /** Golește istoricul din memorie (nu modifică fișierul). */
    void goleste() { m_istoric.clear(); }

    IStocare *storage() const { return m_storage; }

    void setStorage(IStocare *storage)
    {
        if (!storage)
            throw std::invalid_argument("setStorage: storage-ul nu poate fi nullptr.");
        m_storage = storage;
    }

    // ── Filtrare ──────────────────────────────────────────────────────────────

    /**
     * Filtrează tranzacțiile după tip (Achizitionare / Vanzare).
     */
    std::vector<Tranzactie<T>> filtreazaDupaTip(TipTranzactie tip) const
    {
        std::vector<Tranzactie<T>> rezultat;
        for (const Tranzactie<T> &t : m_istoric)
            if (t.tip() == tip)
                rezultat.push_back(t);
        return rezultat;
    }

    /**
     * Filtrează tranzacțiile după numele produsului (case-insensitive, parțial).
     */
    std::vector<Tranzactie<T>> filtreazaDupaProdus(const QString &numeProdus) const
    {
        std::vector<Tranzactie<T>> rezultat;
        const QString termen = numeProdus.toLower();
        for (const Tranzactie<T> &t : m_istoric)
            if (t.numeProdus().toLower().contains(termen))
                rezultat.push_back(t);
        return rezultat;
    }

    /**
     * Filtrează tranzacțiile după numele companiei (case-insensitive, parțial).
     */
    std::vector<Tranzactie<T>> filtreazaDupaCompanie(const QString &numeCompanie) const
    {
        std::vector<Tranzactie<T>> rezultat;
        const QString termen = numeCompanie.toLower();
        for (const Tranzactie<T> &t : m_istoric)
            if (t.numeCompanie().toLower().contains(termen))
                rezultat.push_back(t);
        return rezultat;
    }

    /**
     *  Filtrează tranzacțiile dintr-un interval de timp.
     */
    std::vector<Tranzactie<T>> filtreazaDupaInterval(const QDateTime &de,
                                                     const QDateTime &pana) const
    {
        std::vector<Tranzactie<T>> rezultat;
        for (const Tranzactie<T> &t : m_istoric)
            if (t.timestamp() >= de && t.timestamp() <= pana)
                rezultat.push_back(t);
        return rezultat;
    }

    /**
     * Calculează valoarea totală a tranzacțiilor de un anumit tip.
     */
    double valoareTotala(TipTranzactie tip) const
    {
        double total = 0.0;
        for (const Tranzactie<T> &t : m_istoric)
            if (t.tip() == tip)
                total += t.valoareTotala();
        return total;
    }

private:
    IStocare                   *m_storage;
    std::vector<Tranzactie<T>>  m_istoric;
};

// ── Alias convenabil ──────────────────────────────────────────────────────────
#include "../LibrarieModele/Produs.h"
using IstoricTranzactii = TranzactieStorage<Produs>;

#endif // TRANZACTIEMANAGER_H
