#include "JsonStorage.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <stdexcept>

#include <stdexcept>

static constexpr char KEY_PRODUSE[] = "produse";
static constexpr char KEY_TRANZACTII[]  = "tranzactii";

// ── Constructori ──────────────────────────────────────────────────────────────

JsonStorage::JsonStorage(const QString &caleFisier)
    : m_caleFisier(caleFisier)
{}

// ── Selecție fișier ───────────────────────────────────────────────────────────

bool JsonStorage::alegeFisier(QWidget *parent, bool modSalvare)
{
    QString cale;

    if (modSalvare) {
        cale = QFileDialog::getSaveFileName(
            parent,
            QObject::tr("Salvează depozitul"),
            m_caleFisier.isEmpty() ? QDir::homePath() : m_caleFisier,
            QObject::tr("Fișiere JSON (*.json);;Toate fișierele (*)"));
    } else {
        cale = QFileDialog::getOpenFileName(
            parent,
            QObject::tr("Deschide depozitul"),
            m_caleFisier.isEmpty() ? QDir::homePath() : m_caleFisier,
            QObject::tr("Fișiere JSON (*.json);;Toate fișierele (*)"));
    }

    if (cale.isEmpty())
        return false;   // utilizatorul a anulat

    // Asigurăm extensia .json la salvare
    if (modSalvare && !cale.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        cale += QLatin1String(".json");

    m_caleFisier = cale;
    return true;
}

void JsonStorage::setCaleFisier(const QString &caleFisier)
{
    m_caleFisier = caleFisier;
}

QString JsonStorage::caleFisier() const
{
    return m_caleFisier;
}

bool JsonStorage::areCaleFisier() const
{
    return !m_caleFisier.isEmpty();
}

// ── IStorage::salveaza ────────────────────────────────────────────────────────

void JsonStorage::salveaza(const std::unordered_map<QString, Produs> &produse)
{
    verificaCale();
    asiguraDirector();

    QJsonObject produseObj;
    for (const auto &[id, produs] : produse)
        produseObj.insert(id, produs.toJson());

    QJsonObject radacina;
    radacina[KEY_PRODUSE] = produseObj;

    // QSaveFile → scriere atomică (fișier temp + rename)
    QSaveFile fisier(m_caleFisier);
    if (!fisier.open(QIODevice::WriteOnly | QIODevice::Text))
        throw std::runtime_error(
            QString("Nu pot deschide fișierul pentru scriere: %1\nEroare: %2")
                .arg(m_caleFisier, fisier.errorString())
                .toStdString());

    fisier.write(QJsonDocument(radacina).toJson(QJsonDocument::Indented));

    if (!fisier.commit())
        throw std::runtime_error(
            QString("Scriere eșuată (commit): %1").arg(m_caleFisier).toStdString());
}

// ── IStorage::incarca ─────────────────────────────────────────────────────────

std::unordered_map<QString, Produs> JsonStorage::incarca()
{
    verificaCale();

    std::unordered_map<QString, Produs> rezultat;
    if (!exista())
        return rezultat;

    QFile fisier(m_caleFisier);
    if (!fisier.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error(
            QString("Nu pot deschide fișierul pentru citire: %1\nEroare: %2")
                .arg(m_caleFisier, fisier.errorString())
                .toStdString());

    const QByteArray continut = fisier.readAll();
    fisier.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(continut, &err);
    if (err.error != QJsonParseError::NoError)
        throw std::runtime_error(
            QString("JSON invalid în %1: %2 (offset %3)")
                .arg(m_caleFisier, err.errorString(), QString::number(err.offset))
                .toStdString());

    if (!doc.isObject())
        throw std::runtime_error(
            QString("Structură JSON neașteptată în %1.").arg(m_caleFisier).toStdString());

    const QJsonValue valProduse = doc.object().value(KEY_PRODUSE);
    if (valProduse.isUndefined() || !valProduse.isObject())
        return rezultat;

    const QJsonObject produseObj = valProduse.toObject();
    for (auto it = produseObj.constBegin(); it != produseObj.constEnd(); ++it) {
        if (!it.value().isObject()) continue;
        Produs p = Produs::fromJson(it.value().toObject());
        rezultat.emplace(p.id(), std::move(p));
    }

    return rezultat;
}

// TRANZACTII ───────────────────────────────────────────────
void JsonStorage::salveazaTranzactii(const std::vector<TranzactieProdus> &tranzactii)
{
    verificaCale();
    asiguraDirector();

    // Produsele și tranzacțiile sunt stocate în fișiere SEPARATE
    // (m_storagedepozit ≠ m_storagetranzactii), deci nu există nimic de preservat —
    // citirea prealabilă era complet inutilă și dubla costul fiecărei salvări.
    QJsonArray array;
    for (const TranzactieProdus &t : tranzactii)
        array.append(t.toJson());

    QJsonObject radacina;
    radacina[KEY_TRANZACTII] = array;

    QSaveFile fisier(m_caleFisier);
    if (!fisier.open(QIODevice::WriteOnly | QIODevice::Text))
        throw std::runtime_error(
            QString("Nu pot deschide fișierul pentru scriere: %1\nEroare: %2")
                .arg(m_caleFisier, fisier.errorString())
                .toStdString());

    fisier.write(QJsonDocument(radacina).toJson(QJsonDocument::Indented));

    if (!fisier.commit())
        throw std::runtime_error(
            QString("Scriere eșuată (commit): %1").arg(m_caleFisier).toStdString());
}

std::vector<TranzactieProdus> JsonStorage::incarcaTranzactii()
{
    verificaCale();

    std::vector<TranzactieProdus> rezultat;
    if (!exista())
        return rezultat;

    QFile fisier(m_caleFisier);
    if (!fisier.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error(
            QString("Nu pot deschide fișierul pentru citire: %1\nEroare: %2")
                .arg(m_caleFisier, fisier.errorString())
                .toStdString());

    const QByteArray continut = fisier.readAll();
    fisier.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(continut, &err);
    if (err.error != QJsonParseError::NoError)
        throw std::runtime_error(
            QString("JSON invalid în %1: %2 (offset %3)")
                .arg(m_caleFisier, err.errorString(), QString::number(err.offset))
                .toStdString());

    if (!doc.isObject())
        throw std::runtime_error(
            QString("Structură JSON neașteptată în %1.").arg(m_caleFisier).toStdString());

    const QJsonValue valArray = doc.object().value(KEY_TRANZACTII);
    if (valArray.isUndefined() || !valArray.isArray())
        return rezultat;

    for (const QJsonValue &val : valArray.toArray()) {
        if (!val.isObject()) continue;
        rezultat.push_back(TranzactieProdus::fromJson(val.toObject()));
    }

    return rezultat;
}

// ── IStorage::exista ──────────────────────────────────────────────────────────

bool JsonStorage::exista() const
{
    return !m_caleFisier.isEmpty() && QFileInfo::exists(m_caleFisier);
}

// ── Privat ────────────────────────────────────────────────────────────────────

void JsonStorage::verificaCale() const
{
    if (m_caleFisier.isEmpty())
        throw std::runtime_error(
            "Niciun fișier de stocare selectat. "
            "Apelați alegeFisier() sau setCaleFisier() înainte de a utiliza storage-ul.");
}

void JsonStorage::asiguraDirector() const
{
    const QDir dir = QFileInfo(m_caleFisier).absoluteDir();
    if (!dir.exists() && !QDir().mkpath(dir.absolutePath()))
        throw std::runtime_error(
            QString("Nu pot crea directorul: %1")
                .arg(dir.absolutePath()).toStdString());
}
