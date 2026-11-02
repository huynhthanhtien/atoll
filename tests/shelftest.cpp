// SPDX-FileCopyrightText: 2026 The Atoll contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "shelf/shelfservice.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class ShelfTest : public QObject
{
    Q_OBJECT

    QTemporaryDir m_dir;

    QString touch(const QString &name)
    {
        const QString path = m_dir.filePath(name);
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        return path;
    }

    static QString nameAt(const ShelfService &shelf, int row)
    {
        return shelf.data(shelf.index(row), ShelfService::NameRole).toString();
    }

private Q_SLOTS:
    void addsLocalFilesOnce()
    {
        ShelfService shelf;
        QSignalSpy counted(&shelf, &ShelfService::countChanged);
        const QString path = touch("a.txt");

        shelf.addPaths({path, path});
        shelf.addPaths({path});

        QCOMPARE(shelf.count(), 1);
        QCOMPARE(counted.size(), 1);
        QCOMPARE(nameAt(shelf, 0), QStringLiteral("a.txt"));
    }

    void refusesWhatIsNotAFile()
    {
        ShelfService shelf;
        shelf.add({QUrl(QStringLiteral("https://example.com/a.txt"))});
        shelf.addPaths({m_dir.filePath(QStringLiteral("missing.txt"))});
        QCOMPARE(shelf.count(), 0);
    }

    void removesByRow()
    {
        ShelfService shelf;
        shelf.addPaths({touch("a.txt"), touch("b.txt"), touch("c.txt")});

        shelf.remove(1);
        shelf.remove(-1);
        shelf.remove(9);

        QCOMPARE(shelf.count(), 2);
        QCOMPARE(nameAt(shelf, 0), QStringLiteral("a.txt"));
        QCOMPARE(nameAt(shelf, 1), QStringLiteral("c.txt"));
    }

    void clearEmpties()
    {
        ShelfService shelf;
        shelf.addPaths({touch("a.txt"), touch("b.txt")});
        QSignalSpy counted(&shelf, &ShelfService::countChanged);

        shelf.clear();
        shelf.clear();

        QCOMPARE(shelf.count(), 0);
        QCOMPARE(counted.size(), 1);
        QVERIFY(shelf.leadIcon().isEmpty());
    }

    void prunesFilesThatWentAway()
    {
        ShelfService shelf;
        const QString gone = touch("gone.txt");
        shelf.addPaths({touch("kept.txt"), gone});

        QFile::remove(gone);
        shelf.prune();

        QCOMPARE(shelf.count(), 1);
        QCOMPARE(nameAt(shelf, 0), QStringLiteral("kept.txt"));
    }

    void leadIconFollowsTheNewest()
    {
        ShelfService shelf;
        shelf.addPaths({touch("a.txt"), touch("b.png")});
        const auto icon = shelf.data(shelf.index(1), ShelfService::IconRole).toStringList();
        QCOMPARE(shelf.leadIcon(), icon);
    }
};

QTEST_GUILESS_MAIN(ShelfTest)
#include "shelftest.moc"
