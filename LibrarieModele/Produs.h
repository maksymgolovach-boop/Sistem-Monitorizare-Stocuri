#ifndef PRODUS_H
#define PRODUS_H

#include <QJsonObject>
#include <QString>

class Produs
{
public:
    // ── Constructori ──────────────────────────────────────────────────────────
    Produs();
    explicit Produs(const QString &nume,
                    double         pret,
                    int            cantitate  = 0,
                    int            pragAlerta = 0,
                    const QString &categorie  = "Necategorizat");

    // Constructor de copiere
    Produs(const Produs &other);

    // Operator de atribuire
    Produs &operator=(const Produs &other);

    // Destructor
    ~Produs() = default;

    // ── Getteri ───────────────────────────────────────────────────────────────
    QString nume()       const;
    QString id()         const;
    int     cantitate()  const;
    double  pret()       const;
    int     pragAlerta() const;
    QString categorie()  const;

    // ── Setteri ───────────────────────────────────────────────────────────────
    void setNume      (const QString &nume);
    void setCantitate (int cantitate);
    void setPret      (double pret);
    void setPragAlerta(int pragAlerta);
    void setCategorie (const QString &categorie);
    // ID-ul nu are setter — se generează automat și nu se modifică din exterior

    // ── Logică de alertă ──────────────────────────────────────────────────────
    /** Returnează true dacă stocul a atins sau a coborât sub pragul de alertă. */
    bool esteSubPrag() const;

    // ── Supraîncărcarea operatorilor ──────────────────────────────────────────
    /** Adaugă cantitate la stoc (cantitate trebuie să fie pozitivă). */
    Produs &operator+=(int cantitate);

    /** Scade cantitate din stoc; stocul nu poate deveni negativ. */
    Produs &operator-=(int cantitate);

    // ── Serializare JSON ──────────────────────────────────────────────────────
    QJsonObject toJson()              const;
    static Produs fromJson(const QJsonObject &obj);

    // ── Operator de afișare (util la debug) ───────────────────────────────────
    friend QDebug operator<<(QDebug dbg, const Produs &p);

private:
    QString m_id;          // UUID generat automat
    QString m_nume;
    int     m_cantitate;
    double  m_pret;
    int     m_pragAlerta;
    QString m_categorie;

    void genereazaId();
};

#endif // PRODUS_H
