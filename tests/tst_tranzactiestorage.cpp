#include <QtTest>
#include <stdexcept>

#include "Produs.h"
#include "Tranzactie.h"
#include "Tranzactiemanager.h"
#include "mock_stocare.h"

class TstTranzactieStorage : public QObject
{
    Q_OBJECT

private:
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

    // ── REGRESSION: adauga() NU auto-salveaza ────────────────────────────────

    void adaugaDoesNotAutoSave()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie());
        // saveTranzactiiCount trebuie sa fie 0 — auto-save-ul a fost eliminat
        QCOMPARE(mock.saveTranzactiiCount, 0);
    }

    // ── adauga adauga in memorie ──────────────────────────────────────────────

    void adaugaIncreasesNumar()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        QCOMPARE(ist.numar(), 0);
        ist.adauga(makeTranzactie());
        QCOMPARE(ist.numar(), 1);
        ist.adauga(makeTranzactie());
        QCOMPARE(ist.numar(), 2);
    }

    // ── salveaza() apeleaza storage ───────────────────────────────────────────

    void salveazaCallsStorageOnce()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie());
        ist.salveaza();
        QCOMPARE(mock.saveTranzactiiCount, 1);
    }

    // ── incarcaDate ───────────────────────────────────────────────────────────

    void incarcaDateLoadsFromStorage()
    {
        MockStocare mock;
        mock.dateTranzactii.push_back(makeTranzactie("Oua", TipTranzactie::Vanzare, "ClientA", 10, 0.50));
        mock.dateTranzactii.push_back(makeTranzactie("Faina", TipTranzactie::Achizitionare, "FurnizorB", 20, 2.00));

        IstoricTranzactii ist(&mock);
        ist.incarcaDate();
        QCOMPARE(ist.numar(), 2);
    }

    // ── filtreazaDupaTip ──────────────────────────────────────────────────────

    void filtreazaDupaTipAchizitionareReturnsOnlyAchizitionare()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie("A", TipTranzactie::Achizitionare, "F1", 5, 1.0));
        ist.adauga(makeTranzactie("B", TipTranzactie::Vanzare,       "C1", 3, 2.0));
        ist.adauga(makeTranzactie("C", TipTranzactie::Achizitionare, "F2", 7, 1.5));

        auto result = ist.filtreazaDupaTip(TipTranzactie::Achizitionare);
        QCOMPARE((int)result.size(), 2);
        for (const auto &t : result)
            QCOMPARE(t.tip(), TipTranzactie::Achizitionare);
    }

    void filtreazaDupaTipVanzareReturnsOnlyVanzare()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie("A", TipTranzactie::Achizitionare, "F1", 5, 1.0));
        ist.adauga(makeTranzactie("B", TipTranzactie::Vanzare,       "C1", 3, 2.0));

        auto result = ist.filtreazaDupaTip(TipTranzactie::Vanzare);
        QCOMPARE((int)result.size(), 1);
        QCOMPARE(result[0].tip(), TipTranzactie::Vanzare);
    }

    // ── filtreazaDupaProdus ───────────────────────────────────────────────────

    void filtreazaDupaProdusCaseInsensitivePartialMatch()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie("Lapte integral", TipTranzactie::Achizitionare, "F1", 5, 3.0));
        ist.adauga(makeTranzactie("Branza telemea", TipTranzactie::Vanzare,       "C1", 2, 10.0));
        ist.adauga(makeTranzactie("Lapte degresat",  TipTranzactie::Achizitionare, "F2", 8, 2.5));

        auto result = ist.filtreazaDupaProdus("LAPTE");
        QCOMPARE((int)result.size(), 2);
    }

    // ── filtreazaDupaCompanie ─────────────────────────────────────────────────

    void filtreazaDupaCompaniePartialMatch()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie("P1", TipTranzactie::Achizitionare, "FurnizorAlpha", 5, 1.0));
        ist.adauga(makeTranzactie("P2", TipTranzactie::Achizitionare, "FurnizorBeta",  3, 2.0));
        ist.adauga(makeTranzactie("P3", TipTranzactie::Vanzare,       "ClientGamma",   2, 3.0));

        auto result = ist.filtreazaDupaCompanie("furnizor");
        QCOMPARE((int)result.size(), 2);
    }

    // ── filtreazaDupaInterval ─────────────────────────────────────────────────

    void filtreazaDupaIntervalIncludesInsideInterval()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);

        TranzactieProdus t = makeTranzactie("Lapte", TipTranzactie::Achizitionare, "F1", 5, 1.0);
        ist.adauga(t);

        // Interval larg in jurul timestamp-ului tranzactiei
        QDateTime de   = t.timestamp().addSecs(-60);
        QDateTime pana = t.timestamp().addSecs(60);

        auto result = ist.filtreazaDupaInterval(de, pana);
        QCOMPARE((int)result.size(), 1);
    }

    void filtreazaDupaIntervalExcludesOutsideInterval()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);

        TranzactieProdus t = makeTranzactie("Lapte", TipTranzactie::Achizitionare, "F1", 5, 1.0);
        ist.adauga(t);

        // Interval in trecut (inainte de tranzactie)
        QDateTime de   = t.timestamp().addDays(-2);
        QDateTime pana = t.timestamp().addDays(-1);

        auto result = ist.filtreazaDupaInterval(de, pana);
        QCOMPARE((int)result.size(), 0);
    }

    // ── valoareTotala ─────────────────────────────────────────────────────────

    void valoareTotalaAchizitionareSumsCorrectly()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie("A", TipTranzactie::Achizitionare, "F1", 4, 2.50));  // 10.00
        ist.adauga(makeTranzactie("B", TipTranzactie::Achizitionare, "F2", 3, 1.00));  //  3.00
        ist.adauga(makeTranzactie("C", TipTranzactie::Vanzare,       "C1", 2, 5.00));  // nu se numara

        double total = ist.valoareTotala(TipTranzactie::Achizitionare);
        QVERIFY(qAbs(total - 13.00) < 1e-9);
    }

    void valoareTotalaVanzareSumsCorrectly()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie("A", TipTranzactie::Vanzare, "C1", 5, 3.00));   // 15.00
        ist.adauga(makeTranzactie("B", TipTranzactie::Vanzare, "C2", 2, 10.00));  // 20.00
        ist.adauga(makeTranzactie("C", TipTranzactie::Achizitionare, "F1", 1, 1.00)); // nu se numara

        double total = ist.valoareTotala(TipTranzactie::Vanzare);
        QVERIFY(qAbs(total - 35.00) < 1e-9);
    }

    // ── numar / esteGol / goleste ─────────────────────────────────────────────

    void numarCorrectCount()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        QCOMPARE(ist.numar(), 0);
        ist.adauga(makeTranzactie());
        ist.adauga(makeTranzactie());
        ist.adauga(makeTranzactie());
        QCOMPARE(ist.numar(), 3);
    }

    void esteGolTrueWhenEmpty()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        QVERIFY(ist.esteGol());
    }

    void esteGolFalseWhenNotEmpty()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie());
        QVERIFY(!ist.esteGol());
    }

    void golesteClearsMemory()
    {
        MockStocare mock;
        IstoricTranzactii ist(&mock);
        ist.adauga(makeTranzactie());
        ist.adauga(makeTranzactie());
        ist.goleste();
        QCOMPARE(ist.numar(), 0);
        QVERIFY(ist.esteGol());
        // goleste() nu trebuie sa apeleze storage
        QCOMPARE(mock.saveTranzactiiCount, 0);
    }
};

QTEST_GUILESS_MAIN(TstTranzactieStorage)
#include "tst_tranzactiestorage.moc"
