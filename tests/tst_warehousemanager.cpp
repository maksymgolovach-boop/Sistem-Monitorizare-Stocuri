#include <QtTest>
#include <stdexcept>

#include "Produs.h"
#include "warehousemanager.h"
#include "mock_stocare.h"

class TstWarehouseManager : public QObject
{
    Q_OBJECT

private slots:

    // ── adaugaProdus ──────────────────────────────────────────────────────────

    void adaugaProdusReturnsNonEmptyId()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Oua", 0.50, 60, 10, "Alimente");
        QString id = wm.adaugaProdus(p);
        QVERIFY(!id.isEmpty());
        QCOMPARE(id, p.id());
    }

    void adaugaProdusDuplicateIdThrows()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Oua", 0.50, 60, 10, "Alimente");
        wm.adaugaProdus(p);

        bool threw = false;
        try {
            wm.adaugaProdus(p);   // acelasi ID
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── eliminaProdus ─────────────────────────────────────────────────────────

    void eliminaProdusExistingReturnsTrue()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Zahar", 4.00, 50, 5);
        QString id = wm.adaugaProdus(p);
        QVERIFY(wm.eliminaProdus(id));
    }

    void eliminaProdusNonExistingReturnsFalse()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        QVERIFY(!wm.eliminaProdus("id-inexistent"));
    }

    // ── gasesteProdusDupaId ────────────────────────────────────────────────────

    void gasesteProdusDupaIdFoundReturnsPointer()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Faina", 2.00, 40, 8, "Alimente");
        QString id = wm.adaugaProdus(p);

        const Produs *result = wm.gasesteProdusDupaId(id);
        QVERIFY(result != nullptr);
        QCOMPARE(result->id(), id);
        QCOMPARE(result->nume(), QString("Faina"));
    }

    void gasesteProdusDupaIdNotFoundReturnsNullptr()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        const Produs *result = wm.gasesteProdusDupaId("id-inexistent");
        QVERIFY(result == nullptr);
    }

    // ── actualizeazaProdus ────────────────────────────────────────────────────

    void actualizeazaProdusUpdatesCorrectly()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Ulei", 8.00, 20, 3, "Alimente");
        wm.adaugaProdus(p);

        Produs actualizat(p);  // pastreaza acelasi ID
        actualizat.setNume("Ulei de floarea soarelui");
        actualizat.setPret(9.50);
        wm.actualizeazaProdus(actualizat);

        const Produs *result = wm.gasesteProdusDupaId(p.id());
        QVERIFY(result != nullptr);
        QCOMPARE(result->nume(), QString("Ulei de floarea soarelui"));
        QCOMPARE(result->pret(), 9.50);
        QCOMPARE(result->id(),   p.id());
    }

    void actualizeazaProdusNonExistingThrows()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Produse Inexistent", 1.00);

        bool threw = false;
        try {
            wm.actualizeazaProdus(p);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── adaugaCantitate ───────────────────────────────────────────────────────

    void adaugaCantitateAddsCorrectly()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Sare", 1.00, 10, 2, "Condimente");
        QString id = wm.adaugaProdus(p);
        wm.adaugaCantitate(id, 5);
        const Produs *result = wm.gasesteProdusDupaId(id);
        QCOMPARE(result->cantitate(), 15);
    }

    // ── scadeCantitate ────────────────────────────────────────────────────────

    void scadeCantitateSubtractsCorrectly()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Piper", 3.00, 20, 2, "Condimente");
        QString id = wm.adaugaProdus(p);
        wm.scadeCantitate(id, 8);
        const Produs *result = wm.gasesteProdusDupaId(id);
        QCOMPARE(result->cantitate(), 12);
    }

    void scadeCantitateBelowZeroThrowsUnderflow()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Piper", 3.00, 5, 2, "Condimente");
        QString id = wm.adaugaProdus(p);

        bool threw = false;
        try {
            wm.scadeCantitate(id, 10);
        } catch (const std::underflow_error &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── setCantitate ──────────────────────────────────────────────────────────

    void setCantitateNegativeThrows()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Miere", 25.00, 10, 1, "Dulciuri");
        QString id = wm.adaugaProdus(p);

        bool threw = false;
        try {
            wm.setCantitate(id, -5);
        } catch (const std::invalid_argument &) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── produseSubPrag ────────────────────────────────────────────────────────

    void produseSubPragDetectsCorrectly()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        // produs cu stoc sub prag
        Produs subPrag("Unt", 5.00, 2, 5, "Lactate");
        // produs cu stoc peste prag
        Produs pestePrag("Branza", 12.00, 30, 5, "Lactate");

        wm.adaugaProdus(subPrag);
        wm.adaugaProdus(pestePrag);

        auto lista = wm.produseSubPrag();
        QCOMPARE(lista.size(), std::vector<Produs>::size_type(1));
        QCOMPARE(lista[0].id(), subPrag.id());
    }

    // ── existaAlerte ──────────────────────────────────────────────────────────

    void existaAlerteTrueWhenAlertsExist()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Suc", 3.00, 1, 5, "Bauturi");
        wm.adaugaProdus(p);
        QVERIFY(wm.existaAlerte());
    }

    void existaAlerteFalseWhenNoAlerts()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p("Suc", 3.00, 100, 5, "Bauturi");
        wm.adaugaProdus(p);
        QVERIFY(!wm.existaAlerte());
    }

    void existaAlerteFalseWhenEmpty()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        QVERIFY(!wm.existaAlerte());
    }

    // ── REGRESSION: ValoareProduse cu decimale ────────────────────────────────

    void valoareProduseWithDecimalsNotTruncated()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        // 5 * 2.99 = 14.95; daca acumulatorul era int, rezulta 14
        Produs p("Produs", 2.99, 5, 0, "Test");
        wm.adaugaProdus(p);

        double val = wm.ValoareProduse();
        // Trebuie sa fie 14.95, nu 14.0
        QVERIFY(qAbs(val - 14.95) < 1e-9);
    }

    void valoareProduseMultipleProduse()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        Produs p1("A", 2.50, 4, 0);  // 10.00
        Produs p2("B", 1.99, 3, 0);  // 5.97
        wm.adaugaProdus(p1);
        wm.adaugaProdus(p2);
        double expected = 4 * 2.50 + 3 * 1.99;
        QVERIFY(qAbs(wm.ValoareProduse() - expected) < 1e-9);
    }

    // ── numarProduse / esteGol ────────────────────────────────────────────────

    void numarProduseCount()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        QCOMPARE(wm.numarProduse(), 0);
        wm.adaugaProdus(Produs("P1", 1.00));
        wm.adaugaProdus(Produs("P2", 2.00));
        QCOMPARE(wm.numarProduse(), 2);
    }

    void esteGolTrueWhenEmpty()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        QVERIFY(wm.esteGol());
    }

    void esteGolFalseWhenNotEmpty()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        wm.adaugaProdus(Produs("P1", 1.00));
        QVERIFY(!wm.esteGol());
    }

    // ── incarcaDate / salveazaDate ────────────────────────────────────────────

    void incarcaDateLoadsFromStorage()
    {
        MockStocare mock;
        Produs p("Cafea", 15.00, 10, 2, "Bauturi");
        mock.dateProduse.emplace(p.id(), p);

        WarehouseManager wm(&mock);
        wm.incarcaDate();

        QCOMPARE(wm.numarProduse(), 1);
        const Produs *result = wm.gasesteProdusDupaId(p.id());
        QVERIFY(result != nullptr);
        QCOMPARE(result->nume(), QString("Cafea"));
    }

    void salveazaDateCallsStorage()
    {
        MockStocare mock;
        WarehouseManager wm(&mock);
        wm.adaugaProdus(Produs("Ceai", 5.00, 20, 3));
        wm.salveazaDate();
        QCOMPARE(mock.saveProduseCount, 1);
    }
};

QTEST_GUILESS_MAIN(TstWarehouseManager)
#include "tst_warehousemanager.moc"
