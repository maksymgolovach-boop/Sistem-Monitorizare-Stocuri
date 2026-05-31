#include <QtTest>
#include <stdexcept>

#include "Produs.h"
#include "Tranzactie.h"

class TstTranzactie : public QObject
{
    Q_OBJECT

private:
    // Helper: creeaza un Produs simplu pentru payload
    static Produs makeProdus()
    {
        return Produs("Lapte", 3.50, 100, 10, "Lactate");
    }

private slots:

    // ── Constructor ───────────────────────────────────────────────────────────

    void constructorSetsAllFields()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 20, 3.50, payload);

        QVERIFY(!t.id().isEmpty());
        QCOMPARE(t.numeProdus(),   QString("Lapte"));
        QCOMPARE(t.tip(),          TipTranzactie::Achizitionare);
        QCOMPARE(t.numeCompanie(), QString("FurnizorX"));
        QCOMPARE(t.cantitate(),    20);
        QCOMPARE(t.pretUnitar(),   3.50);
        QVERIFY(t.timestamp().isValid());
    }

    void constructorEmptyNumeProdusThrows()
    {
        Produs payload = makeProdus();
        bool threw = false;
        try {
            TranzactieProdus t("", TipTranzactie::Achizitionare,
                               "FurnizorX", 5, 1.0, payload);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void constructorWhitespaceNumeProdusThrows()
    {
        Produs payload = makeProdus();
        bool threw = false;
        try {
            TranzactieProdus t("   ", TipTranzactie::Achizitionare,
                               "FurnizorX", 5, 1.0, payload);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void constructorEmptyNumeCompanieThrows()
    {
        Produs payload = makeProdus();
        bool threw = false;
        try {
            TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                               "", 5, 1.0, payload);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void constructorZeroCantitateThrows()
    {
        Produs payload = makeProdus();
        bool threw = false;
        try {
            TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                               "FurnizorX", 0, 1.0, payload);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void constructorNegativeCantitateThrows()
    {
        Produs payload = makeProdus();
        bool threw = false;
        try {
            TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                               "FurnizorX", -1, 1.0, payload);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void constructorNegativePretUnitarThrows()
    {
        Produs payload = makeProdus();
        bool threw = false;
        try {
            TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                               "FurnizorX", 5, -0.01, payload);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── valoareTotala ─────────────────────────────────────────────────────────

    void valoareTotalaEqualsCantitateMulPretUnitar()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 4, 2.50, payload);
        QCOMPARE(t.valoareTotala(), 4 * 2.50);
    }

    // ── tipToString / tipFromString ───────────────────────────────────────────

    void tipToStringAchizitionare()
    {
        QCOMPARE(tipToString(TipTranzactie::Achizitionare), QString("Achizitionare"));
    }

    void tipToStringVanzare()
    {
        QCOMPARE(tipToString(TipTranzactie::Vanzare), QString("Vanzare"));
    }

    void tipFromStringUnknownThrows()
    {
        bool threw = false;
        try {
            tipFromString("Inexistent");
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void tipFromStringAchizitionare()
    {
        QCOMPARE(tipFromString("Achizitionare"), TipTranzactie::Achizitionare);
    }

    void tipFromStringVanzare()
    {
        QCOMPARE(tipFromString("Vanzare"), TipTranzactie::Vanzare);
    }

    // ── JSON roundtrip ────────────────────────────────────────────────────────

    void toJsonFromJsonRoundtrip()
    {
        Produs payload = makeProdus();
        TranzactieProdus original("Lapte", TipTranzactie::Vanzare,
                                  "ClientY", 7, 3.50, payload);

        QJsonObject json    = original.toJson();
        TranzactieProdus t2 = TranzactieProdus::fromJson(json);

        QCOMPARE(t2.id(),            original.id());
        QCOMPARE(t2.numeProdus(),    original.numeProdus());
        QCOMPARE(t2.tip(),           original.tip());
        QCOMPARE(t2.numeCompanie(),  original.numeCompanie());
        QCOMPARE(t2.cantitate(),     original.cantitate());
        QCOMPARE(t2.pretUnitar(),    original.pretUnitar());
        // Timestamp serialized via Qt::ISODate (1s precision)
        QCOMPARE(t2.timestamp().toString(Qt::ISODate),
                 original.timestamp().toString(Qt::ISODate));
    }

    // ── Setteri ───────────────────────────────────────────────────────────────

    void setNumeProdusEmptyThrows()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 5, 1.0, payload);
        bool threw = false;
        try {
            t.setNumeProdus("");
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setNumeCompanieEmptyThrows()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 5, 1.0, payload);
        bool threw = false;
        try {
            t.setNumeCompanie("");
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setCantitateZeroThrows()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 5, 1.0, payload);
        bool threw = false;
        try {
            t.setCantitate(0);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setCantitateNegativeThrows()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 5, 1.0, payload);
        bool threw = false;
        try {
            t.setCantitate(-3);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setPretUnitarNegativeThrows()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 5, 1.0, payload);
        bool threw = false;
        try {
            t.setPretUnitar(-1.0);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setTipChangesValue()
    {
        Produs payload = makeProdus();
        TranzactieProdus t("Lapte", TipTranzactie::Achizitionare,
                           "FurnizorX", 5, 1.0, payload);
        t.setTip(TipTranzactie::Vanzare);
        QCOMPARE(t.tip(), TipTranzactie::Vanzare);
    }
};

QTEST_GUILESS_MAIN(TstTranzactie)
#include "tst_tranzactie.moc"
