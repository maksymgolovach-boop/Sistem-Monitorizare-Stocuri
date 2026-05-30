#ifndef WAREHOUSEMANAGER_H
#define WAREHOUSEMANAGER_H

#include "../ManagerStocare/IStocare.h"
#include "../LibrarieModele/Produs.h"

#include <QString>
#include <QStringList>
#include <unordered_map>
#include <vector>

/**
 * @brief Gestionează catalogul de produse al unui depozit.
 *
 * WarehouseManager este complet decuplat de I/O: primește un IStorage*
 * prin constructor și nu știe nimic despre JSON, SQLite sau orice alt format.
 *
 * Responsabilități:
 *  - CRUD pe unordered_map<id, Produs>
 *  - Modificarea cantității cu validare (operatori += / -=)
 *  - Detectarea produselor sub pragul de alertă
 *  - Delegarea persistenței către IStorage
 */
class WarehouseManager
{
public:
    // ── Constructor / Destructor ──────────────────────────────────────────────

    /**
     * @param storage  Pointer la implementarea de stocare (nu poate fi nullptr).
     *                 WarehouseManager nu preia ownership-ul pointerului.
     */
    explicit WarehouseManager(IStocare *storage);
    ~WarehouseManager() = default;

    // Non-copiabil (conține stare mutabilă + pointer extern)
    WarehouseManager(const WarehouseManager &)            = delete;
    WarehouseManager &operator=(const WarehouseManager &) = delete;

    // ── Persistență ───────────────────────────────────────────────────────────

    /**
     * @brief Încarcă toate produsele din storage în memoria internă.
     *        Suprascrie orice date nesalvate din sesiunea curentă.
     * @throws std::runtime_error dacă storage-ul raportează eroare de I/O.
     */
    void incarcaDate();

    /**
     * @brief Salvează starea curentă a catalogului prin storage.
     * @throws std::runtime_error dacă storage-ul raportează eroare de I/O.
     */
    void salveazaDate();

    // ── CRUD produse ──────────────────────────────────────────────────────────

    /**
     * @brief Adaugă un produs nou în catalog.
     * @return ID-ul generat al produsului adăugat.
     * @throws std::invalid_argument dacă un produs cu același ID există deja.
     */
    QString adaugaProdus(const Produs &produs);

    /**
     * @brief Elimină produsul cu ID-ul dat.
     * @return true dacă produsul a existat și a fost eliminat.
     */
    bool eliminaProdus(const QString &id);

    /**
     * @brief Caută un produs după ID.
     * @return Pointer const la produs, sau nullptr dacă nu există.
     */
    const Produs *gasesteProdusDupaId(const QString &id) const;

    /**
     * @brief Caută produse după nume (case-insensitive, potrivire parțială).
     * @return Vector cu copii ale produselor găsite.
     */
    std::vector<Produs> cautaDupaNume(const QString &numePretins) const;

    /**
     * @brief Actualizează datele unui produs existent (fără a-i schimba ID-ul).
     * @throws std::invalid_argument dacă produsul nu există.
     */
    void actualizeazaProdus(const Produs &produsActualizat);

    // ── Modificarea cantității ─────────────────────────────────────────────────

    /**
     * @brief Adaugă cantitate la stocul produsului cu ID-ul dat (operator +=).
     * @param id         ID-ul produsului.
     * @param cantitate  Cantitate pozitivă de adăugat.
     * @throws std::invalid_argument dacă produsul nu există sau cantitatea e negativă.
     */
    void adaugaCantitate(const QString &id, int cantitate);

    /**
     * @brief Scade cantitate din stocul produsului cu ID-ul dat (operator -=).
     * @param id         ID-ul produsului.
     * @param cantitate  Cantitate pozitivă de scăzut.
     * @throws std::invalid_argument  dacă produsul nu există sau cantitatea e negativă.
     * @throws std::underflow_error   dacă stocul ar deveni negativ.
     */
    void scadeCantitate(const QString &id, int cantitate);

    /**
     * @brief Setează direct cantitatea unui produs (înlocuire completă).
     * @throws std::invalid_argument dacă produsul nu există sau cantitatea e negativă.
     */
    void setCantitate(const QString &id, int cantitate);

    // ── Verificare praguri ────────────────────────────────────────────────────

    std::vector<Produs> produseSubPrag() const;

    std::vector<Produs> produseSubPragMinim(int pragMinim) const;

    bool existaAlerte() const;

    QStringList numeProduseSubPrag() const;

    // ── Acces general ─────────────────────────────────────────────────────────

    const std::unordered_map<QString, Produs> &produse() const;

    int numarProduse() const;
    double ValoareProduse()  const;

    bool esteGol() const;

    IStocare *storage() const;

    void setStorage(IStocare *storage);

private:
    IStocare                             *m_storage;
    std::unordered_map<QString, Produs>   m_produse;

    /** Returnează referință mutabilă; aruncă excepție dacă ID-ul nu există. */
    Produs &gasesteSauArunca(const QString &id);

};

#endif // WAREHOUSEMANAGER_H
