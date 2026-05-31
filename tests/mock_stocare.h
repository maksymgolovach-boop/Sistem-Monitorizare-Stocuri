#ifndef MOCK_STOCARE_H
#define MOCK_STOCARE_H

#include "IStocare.h"
#include "Tranzactie.h"

#include <unordered_map>
#include <vector>

/**
 * @brief MockStocare — implementare de test a interfeței IStocare.
 *
 * Nu scrie nimic pe disc.  Contorizează apelurile și expune datele
 * configurate pentru a fi returnate de incarca() / incarcaTranzactii().
 */
class MockStocare : public IStocare
{
public:
    // ── Contoare de apeluri ───────────────────────────────────────────────────
    int saveProduseCount    = 0;
    int saveTranzactiiCount = 0;

    // ── Date returnate la incarcare ───────────────────────────────────────────
    std::unordered_map<QString, Produs> dateProduse;
    std::vector<TranzactieProdus>       dateTranzactii;

    // ── Comportament configurat ───────────────────────────────────────────────
    bool fisierExista = false;

    // ── Ultima data salvata (pentru verificare) ───────────────────────────────
    std::unordered_map<QString, Produs> ultimeleProduseSalvate;
    std::vector<TranzactieProdus>       ultimeleTranzactiiSalvate;

    // ── IStocare ──────────────────────────────────────────────────────────────

    void salveaza(const std::unordered_map<QString, Produs> &produse) override
    {
        ++saveProduseCount;
        ultimeleProduseSalvate = produse;
    }

    std::unordered_map<QString, Produs> incarca() override
    {
        return dateProduse;
    }

    void salveazaTranzactii(const std::vector<TranzactieProdus> &tranzactii) override
    {
        ++saveTranzactiiCount;
        ultimeleTranzactiiSalvate = tranzactii;
    }

    std::vector<TranzactieProdus> incarcaTranzactii() override
    {
        return dateTranzactii;
    }

    bool exista() const override
    {
        return fisierExista;
    }
};

#endif // MOCK_STOCARE_H
