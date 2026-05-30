#include "warehousemanager.h"

#include <stdexcept>
#include <algorithm>

// ── Constructor ───────────────────────────────────────────────────────────────

WarehouseManager::WarehouseManager(IStocare *storage)
    : m_storage(storage)
{
    if (!m_storage)
        throw std::invalid_argument("WarehouseManager: storage-ul nu poate fi nullptr.");
}

// ── Persistență ───────────────────────────────────────────────────────────────

void WarehouseManager::incarcaDate()
{
    m_produse = m_storage->incarca();
}

void WarehouseManager::salveazaDate()
{
    m_storage->salveaza(m_produse);
}

// ── CRUD produse ──────────────────────────────────────────────────────────────

QString WarehouseManager::adaugaProdus(const Produs &produs)
{
    const QString id = produs.id();
    if (m_produse.count(id))
        throw std::invalid_argument(
            QString("Produsul cu ID-ul '%1' există deja în catalog.").arg(id).toStdString());

    m_produse.emplace(id, produs);
    return id;
}

bool WarehouseManager::eliminaProdus(const QString &id)
{
    return m_produse.erase(id) > 0;
}

const Produs *WarehouseManager::gasesteProdusDupaId(const QString &id) const
{
    auto it = m_produse.find(id);
    return (it != m_produse.end()) ? &it->second : nullptr;
}

std::vector<Produs> WarehouseManager::cautaDupaNume(const QString &numePretins) const
{
    std::vector<Produs> rezultat;
    const QString termen = numePretins.trimmed().toLower();

    for (const auto &[id, produs] : m_produse)
        if (produs.nume().toLower().contains(termen))
            rezultat.push_back(produs);

    // Sortăm alfabetic după nume pentru afișare consistentă
    std::sort(rezultat.begin(), rezultat.end(),
              [](const Produs &a, const Produs &b) {
                  return a.nume().toLower() < b.nume().toLower();
              });

    return rezultat;
}

void WarehouseManager::actualizeazaProdus(const Produs &produsActualizat)
{
    Produs &existent = gasesteSauArunca(produsActualizat.id());
    existent = produsActualizat;   // operator= — ID-ul rămâne același
}

// ── Modificarea cantității ────────────────────────────────────────────────────

void WarehouseManager::adaugaCantitate(const QString &id, int cantitate)
{
    if (cantitate < 0)
        throw std::invalid_argument("Cantitatea adăugată nu poate fi negativă.");

    gasesteSauArunca(id) += cantitate;   // Produs::operator+=
}

void WarehouseManager::scadeCantitate(const QString &id, int cantitate)
{
    if (cantitate < 0)
        throw std::invalid_argument("Cantitatea scăzută nu poate fi negativă.");

    // Produs::operator-= aruncă std::underflow_error dacă stocul ar deveni negativ
    gasesteSauArunca(id) -= cantitate;
}

void WarehouseManager::setCantitate(const QString &id, int cantitate)
{
    if (cantitate < 0)
        throw std::invalid_argument("Cantitatea nu poate fi negativă.");

    gasesteSauArunca(id).setCantitate(cantitate);
}

// ── Verificare praguri ────────────────────────────────────────────────────────

std::vector<Produs> WarehouseManager::produseSubPrag() const
{
    std::vector<Produs> rezultat;

    for (const auto &[id, produs] : m_produse)
        if (produs.esteSubPrag())          // Produs::esteSubPrag() → cantitate <= pragAlerta
            rezultat.push_back(produs);

    std::sort(rezultat.begin(), rezultat.end(),
              [](const Produs &a, const Produs &b) {
                  // Cel mai critic (cel mai departe sub prag) apare primul
                  return a.cantitate() < b.cantitate();
              });

    return rezultat;
}

std::vector<Produs> WarehouseManager::produseSubPragMinim(int pragMinim) const
{
    std::vector<Produs> rezultat;

    for (const auto &[id, produs] : m_produse)
        if (produs.cantitate() <= pragMinim)
            rezultat.push_back(produs);

    std::sort(rezultat.begin(), rezultat.end(),
              [](const Produs &a, const Produs &b) {
                  return a.cantitate() < b.cantitate();
              });

    return rezultat;
}

bool WarehouseManager::existaAlerte() const
{
    for (const auto &[id, produs] : m_produse)
        if (produs.esteSubPrag())
            return true;
    return false;
}

QStringList WarehouseManager::numeProduseSubPrag() const
{
    QStringList lista;
    for (const Produs &p : produseSubPrag())
        lista << QString("%1 (stoc: %2 / prag: %3)")
                     .arg(p.nume())
                     .arg(p.cantitate())
                     .arg(p.pragAlerta());
    return lista;
}

// ── Acces general ─────────────────────────────────────────────────────────────

const std::unordered_map<QString, Produs> &WarehouseManager::produse() const
{
    return m_produse;
}

int WarehouseManager::numarProduse() const
{
    return static_cast<int>(m_produse.size());
}

double WarehouseManager::ValoareProduse() const
{
    double val = 0.0;   // era int → trunchia zecimalele (ex: 5×2.99 = 14, nu 14.95)
    for (const auto &[id, produs] : m_produse)
        val += produs.cantitate() * produs.pret();
    return val;
}

bool WarehouseManager::esteGol() const
{
    return m_produse.empty();
}

IStocare *WarehouseManager::storage() const
{
    return m_storage;
}

void WarehouseManager::setStorage(IStocare *storage)
{
    if (!storage)
        throw std::invalid_argument("setStorage: storage-ul nu poate fi nullptr.");
    m_storage = storage;
}

// ── Privat ────────────────────────────────────────────────────────────────────

Produs &WarehouseManager::gasesteSauArunca(const QString &id)
{
    auto it = m_produse.find(id);
    if (it == m_produse.end())
        throw std::invalid_argument(
            QString("Produsul cu ID-ul '%1' nu există în catalog.").arg(id).toStdString());
    return it->second;
}
