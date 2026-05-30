#ifndef ISTOCARE_H
#define ISTOCARE_H

#include <QString>
#include "../LibrarieModele/Produs.h"
#include "../LibrarieModele/Tranzactie.h"

class IStocare
{
public:
    virtual ~IStocare() = default;

    /**
     * @brief Salvează întregul catalog de produse.
     * @param produse  Map-ul de salvat (id → Produs).
     * @throws std::runtime_error dacă scrierea eșuează.
     */
    virtual void salveaza(const std::unordered_map<QString, Produs> &produse) = 0;

    /**
     * @brief Încarcă catalogul de produse din sursa de stocare.
     * @return Map-ul deserializat (id → Produs).
     * @throws std::runtime_error dacă citirea sau parsarea eșuează.
     */
    virtual std::unordered_map<QString, Produs> incarca() = 0;

    virtual void salveazaTranzactii(const std::vector<TranzactieProdus> &tranzactii) = 0;
    virtual std::vector<TranzactieProdus> incarcaTranzactii() = 0;

    virtual bool exista() const = 0;
};

#endif // ISTOCARE_H
