#include "Produs.h"

#include <QUuid>
#include <QDebug>
#include <stdexcept>

// ── Constructori ──────────────────────────────────────────────────────────────

Produs::Produs()
    : m_cantitate(0)
    , m_pret(0.0)
    , m_pragAlerta(0)
{
    genereazaId();
}

Produs::Produs(const QString &nume,
               double         pret,
               int            cantitate,
               int            pragAlerta,
               const QString &categorie)
    : m_nume      (nume)
    , m_cantitate (cantitate)
    , m_pret      (pret)
    , m_pragAlerta(pragAlerta)
    , m_categorie (categorie.trimmed().isEmpty() ? "Necategorizat" : categorie.trimmed())
{
    genereazaId();
}

// Constructor de copiere — păstrează același ID (aceeași entitate)
Produs::Produs(const Produs &other)
    : m_id        (other.m_id)
    , m_nume      (other.m_nume)
    , m_cantitate (other.m_cantitate)
    , m_pret      (other.m_pret)
    , m_pragAlerta(other.m_pragAlerta)
    , m_categorie (other.m_categorie)
{}

// Operator de atribuire
Produs &Produs::operator=(const Produs &other)
{
    if (this != &other) {
        m_id         = other.m_id;
        m_nume       = other.m_nume;
        m_cantitate  = other.m_cantitate;
        m_pret       = other.m_pret;
        m_pragAlerta = other.m_pragAlerta;
        m_categorie  = other.m_categorie;
    }
    return *this;
}

// ── Getteri ───────────────────────────────────────────────────────────────────

QString Produs::nume()       const { return m_nume;       }
QString Produs::id()         const { return m_id;         }
int     Produs::cantitate()  const { return m_cantitate;  }
double  Produs::pret()       const { return m_pret;       }
int     Produs::pragAlerta() const { return m_pragAlerta; }
QString Produs::categorie()  const { return m_categorie;  }

// ── Setteri ───────────────────────────────────────────────────────────────────

void Produs::setNume(const QString &nume)
{
    if (nume.trimmed().isEmpty())
        throw std::invalid_argument("Numele produsului nu poate fi gol.");
    m_nume = nume.trimmed();
}

void Produs::setCantitate(int cantitate)
{
    if (cantitate < 0)
        throw std::invalid_argument("Cantitatea nu poate fi negativă.");
    m_cantitate = cantitate;
}

void Produs::setPret(double pret)
{
    if (pret < 0.0)
        throw std::invalid_argument("Prețul nu poate fi negativ.");
    m_pret = pret;
}

void Produs::setPragAlerta(int pragAlerta)
{
    if (pragAlerta < 0)
        throw std::invalid_argument("Pragul de alertă nu poate fi negativ.");
    m_pragAlerta = pragAlerta;
}

void Produs::setCategorie(const QString &categorie)
{
    m_categorie = categorie.trimmed().isEmpty() ? "Necategorizat" : categorie.trimmed();
}

// ── Logică de alertă ──────────────────────────────────────────────────────────

bool Produs::esteSubPrag() const
{
    return m_cantitate <= m_pragAlerta;
}

// ── Supraîncărcarea operatorilor ──────────────────────────────────────────────

Produs &Produs::operator+=(int cantitate)
{
    if (cantitate < 0)
        throw std::invalid_argument("Valoarea adăugată trebuie să fie pozitivă. "
                                    "Folosiți -= pentru scădere.");
    m_cantitate += cantitate;
    return *this;
}

Produs &Produs::operator-=(int cantitate)
{
    if (cantitate < 0)
        throw std::invalid_argument("Valoarea scăzută trebuie să fie pozitivă.");
    if (cantitate > m_cantitate)
        throw std::underflow_error("Stocul nu poate deveni negativ.");
    m_cantitate -= cantitate;
    return *this;
}

// ── Serializare JSON ──────────────────────────────────────────────────────────

QJsonObject Produs::toJson() const
{
    QJsonObject obj;
    obj["id"]         = m_id;
    obj["nume"]       = m_nume;
    obj["cantitate"]  = m_cantitate;
    obj["pret"]       = m_pret;
    obj["pragAlerta"] = m_pragAlerta;
    obj["categorie"]  = m_categorie;
    return obj;
}

Produs Produs::fromJson(const QJsonObject &obj)
{
    Produs p;
    // Restaurăm ID-ul existent din fișier în loc să generăm unul nou
    p.m_id         = obj["id"].toString();
    p.m_nume       = obj["nume"].toString();
    p.m_cantitate  = obj["cantitate"].toInt();
    p.m_pret       = obj["pret"].toDouble();
    p.m_pragAlerta = obj["pragAlerta"].toInt();
    // Compatibilitate cu fișierele vechi (fără câmpul "categorie")
    p.m_categorie  = obj["categorie"].toString("Necategorizat");
    return p;
}

// ── Operator de afișare ───────────────────────────────────────────────────────

QDebug operator<<(QDebug dbg, const Produs &p)
{
    dbg.nospace()
    << "Produs("
    << "id="         << p.m_id         << ", "
    << "nume="       << p.m_nume       << ", "
    << "cantitate="  << p.m_cantitate  << ", "
    << "pret="       << p.m_pret       << " RON, "
    << "pragAlerta=" << p.m_pragAlerta << ", "
    << "categorie="  << p.m_categorie
    << ")";
    return dbg;
}

// ── Privat ────────────────────────────────────────────────────────────────────

void Produs::genereazaId()
{
    // QUuid::createUuid() generează un UUID v4 (aleatoriu).
    // Îl convertim la string fără acolade: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}
