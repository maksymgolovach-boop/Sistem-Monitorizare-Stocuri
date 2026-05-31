#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <stdexcept>

#include "Produs.h"
#include "Tranzactie.h"
#include "jsonstorage.h"

class TstJsonStorage : public QObject
{
    Q_OBJECT

private:
    // Helper: calea unui fisier cu un anumit nume in directorul temporar
    static QString tempPath(const QTemporaryDir &dir, const QString &filename)
    {
        return dir.path() + "/" + filename;
    }

    static TranzactieProdus makeTranzactie(
        const QString &numeProdus   = "Lapte",
        TipTranzactie  tip          = TipTranzactie::Achizitionare,
        const QString &numeCompanie = "FurnizorX",
        int            cantitate    = 5,
        double         pretUnitar   = 3.50)
    {
        Produs payload(numeProdus, pretUnitar, cantitate);
        return TranzactieProdus(numeProdus, tip, numeCompanie,
                                cantitate, pretUnitar, payload);
    }

private slots:

    // ── salveaza / incarca roundtrip ──────────────────────────────────────────

    void salveazaIncarcaRoundtripProduse()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        JsonStorage storage(tempPath(dir, "depozit.json"));

        Produs p1("Lapte", 3.50, 100, 10, "Lactate");
        Produs p2("Paine", 2.00, 50,   5, "Panificatie");
        std::unordered_map<QString, Produs> original;
        original.emplace(p1.id(), p1);
        original.emplace(p2.id(), p2);

        storage.salveaza(original);
        auto loaded = storage.incarca();

        QCOMPARE((int)loaded.size(), 2);

        auto it1 = loaded.find(p1.id());
        QVERIFY(it1 != loaded.end());
        QCOMPARE(it1->second.id(),         p1.id());
        QCOMPARE(it1->second.nume(),       p1.nume());
        QCOMPARE(it1->second.cantitate(),  p1.cantitate());
        QCOMPARE(it1->second.pret(),       p1.pret());
        QCOMPARE(it1->second.pragAlerta(), p1.pragAlerta());
        QCOMPARE(it1->second.categorie(),  p1.categorie());

        auto it2 = loaded.find(p2.id());
        QVERIFY(it2 != loaded.end());
        QCOMPARE(it2->second.id(),   p2.id());
        QCOMPARE(it2->second.nume(), p2.nume());
    }

    void salveazaIncarcaRoundtripTranzactii()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        JsonStorage storage(tempPath(dir, "tranzactii.json"));

        TranzactieProdus t1 = makeTranzactie("Lapte", TipTranzactie::Achizitionare, "FurnA", 10, 3.50);
        TranzactieProdus t2 = makeTranzactie("Oua",   TipTranzactie::Vanzare,       "ClientB", 6, 0.50);
        std::vector<TranzactieProdus> original = {t1, t2};

        storage.salveazaTranzactii(original);
        auto loaded = storage.incarcaTranzactii();

        QCOMPARE((int)loaded.size(), 2);
        QCOMPARE(loaded[0].id(),            t1.id());
        QCOMPARE(loaded[0].numeProdus(),    t1.numeProdus());
        QCOMPARE(loaded[0].tip(),           t1.tip());
        QCOMPARE(loaded[0].numeCompanie(),  t1.numeCompanie());
        QCOMPARE(loaded[0].cantitate(),     t1.cantitate());
        QCOMPARE(loaded[0].pretUnitar(),    t1.pretUnitar());
        QCOMPARE(loaded[0].timestamp().toString(Qt::ISODate),
                 t1.timestamp().toString(Qt::ISODate));

        QCOMPARE(loaded[1].id(), t2.id());
    }

    // ── fisier inexistent ─────────────────────────────────────────────────────

    void incarcaOnNonExistingFileReturnsEmptyMap()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        JsonStorage storage(tempPath(dir, "inexistent.json"));
        auto result = storage.incarca();
        QVERIFY(result.empty());
    }

    void incarcaTranzactiiOnNonExistingFileReturnsEmptyVector()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        JsonStorage storage(tempPath(dir, "inexistent_tranz.json"));
        auto result = storage.incarcaTranzactii();
        QVERIFY(result.empty());
    }

    // ── exista() ──────────────────────────────────────────────────────────────

    void existaFalseForNonExistingFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        JsonStorage storage(tempPath(dir, "missing.json"));
        QVERIFY(!storage.exista());
    }

    void existaTrueAfterSave()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        JsonStorage storage(tempPath(dir, "depozit.json"));
        QVERIFY(!storage.exista());

        std::unordered_map<QString, Produs> produse;
        storage.salveaza(produse);
        QVERIFY(storage.exista());
    }

    // ── verificaCale throws when no path ─────────────────────────────────────

    void verificaCaleThrowsWhenNoPathSet()
    {
        JsonStorage storage;  // fara cale
        bool threw = false;
        try {
            storage.incarca();
        } catch (const std::runtime_error &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void verificaCaleThrowsOnSalveazaWhenNoPath()
    {
        JsonStorage storage;
        bool threw = false;
        try {
            std::unordered_map<QString, Produs> empty;
            storage.salveaza(empty);
        } catch (const std::runtime_error &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── compatibilitate cu fisiere vechi fara "categorie" ────────────────────

    void incarcaHandlesOldJsonWithoutCategorieField()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = tempPath(dir, "vechi.json");

        // Scriem manual un JSON fara campul "categorie"
        QJsonObject prodObj;
        prodObj["id"]         = "old-id-001";
        prodObj["nume"]       = "Produs Vechi";
        prodObj["cantitate"]  = 10;
        prodObj["pret"]       = 5.00;
        prodObj["pragAlerta"] = 2;
        // "categorie" lipseste

        QJsonObject produseObj;
        produseObj["old-id-001"] = prodObj;

        QJsonObject root;
        root["produse"] = produseObj;

        QFile f(cale);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write(QJsonDocument(root).toJson());
        f.close();

        JsonStorage storage(cale);
        auto loaded = storage.incarca();
        QCOMPARE((int)loaded.size(), 1);
        auto it = loaded.find("old-id-001");
        QVERIFY(it != loaded.end());
        QCOMPARE(it->second.categorie(), QString("Necategorizat"));
    }

    // ── salveazaTranzactii este write-only (nu citeste fisierul inainte) ──────

    void salveazaTranzactiiDoesNotReadFileFirst()
    {
        // Verificam ca un fisier care contine doar produse (nu tranzactii)
        // nu cauzeaza o eroare cand salvam tranzactii in el.
        // (Fisierele produse si tranzactii sunt SEPARATE.)
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString cale = tempPath(dir, "tranz_only.json");

        JsonStorage storage(cale);

        // Salvam direct tranzactii fara sa fi salvat produse inainte
        std::vector<TranzactieProdus> tranzactii;
        tranzactii.push_back(makeTranzactie());

        // Nu trebuie sa arunce exceptie
        bool threw = false;
        try {
            storage.salveazaTranzactii(tranzactii);
        } catch (...) {
            threw = true;
        }
        QVERIFY(!threw);

        // Fisierul trebuie sa existe si sa contina tranzactia
        QVERIFY(storage.exista());
        auto loaded = storage.incarcaTranzactii();
        QCOMPARE((int)loaded.size(), 1);
    }

    // ── setCaleFisier / caleFisier / areCaleFisier ────────────────────────────

    void setCaleFisierSetsPath()
    {
        JsonStorage storage;
        QVERIFY(!storage.areCaleFisier());
        storage.setCaleFisier("/tmp/test.json");
        QVERIFY(storage.areCaleFisier());
        QCOMPARE(storage.caleFisier(), QString("/tmp/test.json"));
    }
};

QTEST_GUILESS_MAIN(TstJsonStorage)
#include "tst_jsonstorage.moc"
