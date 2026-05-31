#include <QtTest>
#include <stdexcept>

#include "Produs.h"

class TstProdus : public QObject
{
    Q_OBJECT

private slots:

    // ── Constructori ──────────────────────────────────────────────────────────

    void defaultConstructorValues()
    {
        Produs p;
        QVERIFY(!p.id().isEmpty());
        QCOMPARE(p.cantitate(),  0);
        QCOMPARE(p.pret(),       0.0);
        QCOMPARE(p.pragAlerta(), 0);
        // Default constructor leaves m_nume empty and m_categorie is set via
        // the explicit constructor; default-constructed categorie is empty string.
        // (No validation in default ctor, so we just check numeric defaults.)
    }

    void explicitConstructorSetsValues()
    {
        Produs p("Lapte", 3.50, 100, 10, "Lactate");
        QCOMPARE(p.nume(),       QString("Lapte"));
        QCOMPARE(p.pret(),       3.50);
        QCOMPARE(p.cantitate(),  100);
        QCOMPARE(p.pragAlerta(), 10);
        QCOMPARE(p.categorie(),  QString("Lactate"));
        QVERIFY(!p.id().isEmpty());
    }

    void copyConstructorPreservesId()
    {
        Produs original("Paine", 2.00, 50, 5, "Panificatie");
        Produs copie(original);
        QCOMPARE(copie.id(),   original.id());
        QCOMPARE(copie.nume(), original.nume());
        QCOMPARE(copie.pret(), original.pret());
    }

    void assignmentOperator()
    {
        Produs a("Unt", 5.00, 20, 3, "Lactate");
        Produs b;
        b = a;
        QCOMPARE(b.id(),         a.id());
        QCOMPARE(b.nume(),       a.nume());
        QCOMPARE(b.cantitate(),  a.cantitate());
        QCOMPARE(b.pret(),       a.pret());
        QCOMPARE(b.pragAlerta(), a.pragAlerta());
        QCOMPARE(b.categorie(),  a.categorie());
    }

    // ── Setteri cu validare ───────────────────────────────────────────────────

    void setNumeEmptyStringThrows()
    {
        Produs p("Test", 1.0);
        bool threw = false;
        try {
            p.setNume("");
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setNumeWhitespaceThrows()
    {
        Produs p("Test", 1.0);
        bool threw = false;
        try {
            p.setNume("   ");
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setCantitateNegativeThrows()
    {
        Produs p("Test", 1.0, 10);
        bool threw = false;
        try {
            p.setCantitate(-1);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setPretNegativeThrows()
    {
        Produs p("Test", 1.0);
        bool threw = false;
        try {
            p.setPret(-0.01);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setPragAlertaNegativeThrows()
    {
        Produs p("Test", 1.0);
        bool threw = false;
        try {
            p.setPragAlerta(-1);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void setCategorieWhitespaceOnlySetsNecategorizat()
    {
        Produs p("Test", 1.0);
        p.setCategorie("   ");
        QCOMPARE(p.categorie(), QString("Necategorizat"));
    }

    void setCategorieEmptyStringSetsNecategorizat()
    {
        Produs p("Test", 1.0);
        p.setCategorie("");
        QCOMPARE(p.categorie(), QString("Necategorizat"));
    }

    // ── esteSubPrag ───────────────────────────────────────────────────────────

    void esteSubPragBoundaryEqualReturnsTrue()
    {
        // cantitate == prag => true (sub sau egal cu pragul)
        Produs p("Test", 1.0, /*cantitate=*/5, /*prag=*/5);
        QVERIFY(p.esteSubPrag());
    }

    void esteSubPragAboveReturnsFalse()
    {
        Produs p("Test", 1.0, /*cantitate=*/6, /*prag=*/5);
        QVERIFY(!p.esteSubPrag());
    }

    void esteSubPragBelowReturnsTrue()
    {
        Produs p("Test", 1.0, /*cantitate=*/3, /*prag=*/5);
        QVERIFY(p.esteSubPrag());
    }

    // ── operator+= ───────────────────────────────────────────────────────────

    void operatorPlusEqualsAddsCorrectly()
    {
        Produs p("Test", 1.0, 10);
        p += 5;
        QCOMPARE(p.cantitate(), 15);
    }

    void operatorPlusEqualsNegativeThrows()
    {
        Produs p("Test", 1.0, 10);
        bool threw = false;
        try {
            p += -1;
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── operator-= ───────────────────────────────────────────────────────────

    void operatorMinusEqualsSubtractsCorrectly()
    {
        Produs p("Test", 1.0, 10);
        p -= 3;
        QCOMPARE(p.cantitate(), 7);
    }

    void operatorMinusEqualsBelowZeroThrowsUnderflow()
    {
        Produs p("Test", 1.0, 5);
        bool threw = false;
        try {
            p -= 10;
        } catch (const std::underflow_error &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── JSON roundtrip ────────────────────────────────────────────────────────

    void toJsonFromJsonRoundtrip()
    {
        Produs original("Cafea", 12.99, 30, 5, "Bauturi");
        QJsonObject json = original.toJson();
        Produs restored  = Produs::fromJson(json);

        QCOMPARE(restored.id(),         original.id());
        QCOMPARE(restored.nume(),       original.nume());
        QCOMPARE(restored.cantitate(),  original.cantitate());
        QCOMPARE(restored.pret(),       original.pret());
        QCOMPARE(restored.pragAlerta(), original.pragAlerta());
        QCOMPARE(restored.categorie(),  original.categorie());
    }

    void fromJsonMissingCategorieDefaultsToNecategorizat()
    {
        // Simulam un fisier vechi fara campul "categorie"
        QJsonObject obj;
        obj["id"]         = "test-id-123";
        obj["nume"]       = "Produs Vechi";
        obj["cantitate"]  = 10;
        obj["pret"]       = 5.0;
        obj["pragAlerta"] = 2;
        // "categorie" lipseste intentionat

        Produs p = Produs::fromJson(obj);
        QCOMPARE(p.categorie(), QString("Necategorizat"));
    }

    void twoDifferentInstancesHaveDifferentIds()
    {
        Produs a("Produs A", 1.0);
        Produs b("Produs B", 2.0);
        QVERIFY(a.id() != b.id());
    }

    void constructorWhitespaceCategorieBecomesNecategorizat()
    {
        Produs p("Test", 1.0, 0, 0, "   ");
        QCOMPARE(p.categorie(), QString("Necategorizat"));
    }
};

QTEST_GUILESS_MAIN(TstProdus)
#include "tst_produs.moc"
