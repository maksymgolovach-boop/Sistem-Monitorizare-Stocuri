#ifndef TRANZACTIE_H
#define TRANZACTIE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>
#include <QDebug>

#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// TipTranzactie — enum strong-typed pentru tipul operației
// ─────────────────────────────────────────────────────────────────────────────

enum class TipTranzactie {
    Achizitionare,   // Intrare  — stocul crește
    Vanzare          // Ieșire   — stocul scade
};

/** Conversie enum → QString pentru afișare și serializare. */
inline QString tipToString(TipTranzactie tip)
{
    switch (tip) {
    case TipTranzactie::Achizitionare: return QStringLiteral("Achizitionare");
    case TipTranzactie::Vanzare:       return QStringLiteral("Vanzare");
    }
    return QStringLiteral("Necunoscut");
}

/** Conversie QString → enum; aruncă excepție la valoare necunoscută. */
inline TipTranzactie tipFromString(const QString &str)
{
    if (str == QLatin1String("Achizitionare")) return TipTranzactie::Achizitionare;
    if (str == QLatin1String("Vanzare"))       return TipTranzactie::Vanzare;
    throw std::invalid_argument(
        QString("TipTranzactie necunoscut: '%1'").arg(str).toStdString());
}

// ─────────────────────────────────────────────────────────────────────────────
// Tranzactie<T>
//
// Parametrul de tip T reprezintă "payload-ul" tranzacției, adică entitatea
// asupra căreia se operează.  În practică vei instanția cu Produs:
//     Tranzactie<Produs>

template <typename T>
class Tranzactie
{
public:
    // ── Constructori ─────────────────────────────────────────────────────────

    /**
     * @brief Constructor principal.
     * @param numeProdus     Numele produsului implicat în tranzacție.
     * @param tip            Achizitionare (intrare) sau Vanzare (ieșire).
     * @param numeCompanie   Compania parteneră (furnizor / client).
     * @param cantitate      Numărul de unități tranzacționate (pozitiv).
     * @param pretUnitar     Prețul per unitate la momentul tranzacției.
     * @param payload        Copia produsului/entității la momentul tranzacției.
     */
    Tranzactie(const QString      &numeProdus,
               TipTranzactie       tip,
               const QString      &numeCompanie,
               int                 cantitate,
               double              pretUnitar,
               const T            &payload)
        : m_id          (QUuid::createUuid().toString(QUuid::WithoutBraces))
        , m_numeProdus  (numeProdus)
        , m_tip         (tip)
        , m_numeCompanie(numeCompanie)
        , m_cantitate   (cantitate)
        , m_pretUnitar  (pretUnitar)
        , m_timestamp   (QDateTime::currentDateTime())
        , m_payload     (payload)
    {
        valideaza();
    }

    /** Constructor de copiere. */
    Tranzactie(const Tranzactie &other)            = default;

    /** Operator de atribuire. */
    Tranzactie &operator=(const Tranzactie &other) = default;

    /** Destructor. */
    ~Tranzactie() = default;

    // ── Getteri ───────────────────────────────────────────────────────────────

    QString        id()            const { return m_id;           }

    QString        numeProdus()    const { return m_numeProdus;   }

    TipTranzactie  tip()           const { return m_tip;          }

    QString        tipString()     const { return tipToString(m_tip); }

    QString        numeCompanie()  const { return m_numeCompanie; }

    int            cantitate()     const { return m_cantitate;    }

    double         pretUnitar()    const { return m_pretUnitar;   }

    double         valoareTotala() const { return m_cantitate * m_pretUnitar; }

    QDateTime      timestamp()     const { return m_timestamp;    }

    const T       &payload()       const { return m_payload;      }

    // ── Setteri ───────────────────────────────────────────────────────────────
    // ID-ul și timestamp-ul sunt imuabile după creare.

    void setNumeProdus  (const QString &numeProdus)
    {
        if (numeProdus.trimmed().isEmpty())
            throw std::invalid_argument("Numele produsului nu poate fi gol.");
        m_numeProdus = numeProdus.trimmed();
    }

    void setTip         (TipTranzactie tip)           { m_tip          = tip;           }

    void setNumeCompanie(const QString &numeCompanie)
    {
        if (numeCompanie.trimmed().isEmpty())
            throw std::invalid_argument("Numele companiei nu poate fi gol.");
        m_numeCompanie = numeCompanie.trimmed();
    }

    void setCantitate   (int cantitate)
    {
        if (cantitate <= 0)
            throw std::invalid_argument("Cantitatea trebuie să fie strict pozitivă.");
        m_cantitate = cantitate;
    }

    void setPretUnitar  (double pret)
    {
        if (pret < 0.0)
            throw std::invalid_argument("Prețul unitar nu poate fi negativ.");
        m_pretUnitar = pret;
    }

    void setPayload     (const T &payload)            { m_payload      = payload;       }

    // ── Serializare JSON ──────────────────────────────────────────────────────

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj[QStringLiteral("id")]            = m_id;
        obj[QStringLiteral("numeProdus")]    = m_numeProdus;
        obj[QStringLiteral("tip")]           = tipToString(m_tip);
        obj[QStringLiteral("numeCompanie")]  = m_numeCompanie;
        obj[QStringLiteral("cantitate")]     = m_cantitate;
        obj[QStringLiteral("pretUnitar")]    = m_pretUnitar;
        obj[QStringLiteral("valoareTotala")] = valoareTotala();
        obj[QStringLiteral("timestamp")]     = m_timestamp.toString(Qt::ISODate);
        obj[QStringLiteral("payload")]       = m_payload.toJson();
        return obj;
    }

    /**
     * @brief Deserializează o tranzacție dintr-un QJsonObject.
     *
     * Utilizare:
     * @code
     *   auto t = Tranzactie<Produs>::fromJson(jsonObj);
     * @endcode
     */
    static Tranzactie<T> fromJson(const QJsonObject &obj)
    {
        // Reconstruim payload-ul prin metoda statică a lui T
        const T payload = T::fromJson(obj[QStringLiteral("payload")].toObject());

        Tranzactie<T> t(
            obj[QStringLiteral("numeProdus")].toString(),
            tipFromString(obj[QStringLiteral("tip")].toString()),
            obj[QStringLiteral("numeCompanie")].toString(),
            obj[QStringLiteral("cantitate")].toInt(),
            obj[QStringLiteral("pretUnitar")].toDouble(),
            payload
            );

        // Restaurăm ID-ul și timestamp-ul originale (nu generăm altele noi)
        t.m_id        = obj[QStringLiteral("id")].toString();
        t.m_timestamp = QDateTime::fromString(
            obj[QStringLiteral("timestamp")].toString(), Qt::ISODate);

        return t;
    }

private:
    QString       m_id;
    QString       m_numeProdus;
    TipTranzactie m_tip;
    QString       m_numeCompanie;
    int           m_cantitate;
    double        m_pretUnitar;
    QDateTime     m_timestamp;
    T             m_payload;       // snapshot al produsului la momentul tranzacției

    /** Validare internă apelată din constructor. */
    void valideaza() const
    {
        if (m_numeProdus.trimmed().isEmpty())
            throw std::invalid_argument("Numele produsului nu poate fi gol.");
        if (m_numeCompanie.trimmed().isEmpty())
            throw std::invalid_argument("Numele companiei nu poate fi gol.");
        if (m_cantitate <= 0)
            throw std::invalid_argument("Cantitatea trebuie să fie strict pozitivă.");
        if (m_pretUnitar < 0.0)
            throw std::invalid_argument("Prețul unitar nu poate fi negativ.");
    }
};

#include "Produs.h"
using TranzactieProdus = Tranzactie<Produs>;

#endif // TRANZACTIE_H
