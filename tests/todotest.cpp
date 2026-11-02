// SPDX-License-Identifier: GPL-3.0-or-later
#include "todo/todoservice.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTest>
#include <QVariantMap>

class TodoTest : public QObject {
    Q_OBJECT

    static QString path() {
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/todos.json";
    }
    static QVariantMap at(const TodoService &todo, int index) { return todo.items().at(index).toMap(); }

private Q_SLOTS:
    void initTestCase() { QStandardPaths::setTestModeEnabled(true); }
    void init() {
        QFile::remove(path());
        QFile::remove(path() + ".bad");
    }

    void addTidiesAndRejectsBlank() {
        TodoService todo;
        QVERIFY(!todo.add("   \n "));
        QVERIFY(todo.add("  buy   milk "));
        QCOMPARE(todo.items().size(), 1);
        QCOMPARE(at(todo, 0)["text"].toString(), "buy milk");
        QCOMPARE(todo.remaining(), 1);
    }

    void toggleRemoveAndClear() {
        TodoService todo;
        todo.add("one");
        todo.add("two");
        todo.add("three");
        const QString one = at(todo, 0)["id"].toString();
        const QString two = at(todo, 1)["id"].toString();

        todo.toggle(one);
        QVERIFY(at(todo, 0)["done"].toBool());
        QCOMPARE(todo.remaining(), 2);
        todo.toggle(one);
        QCOMPARE(todo.remaining(), 3);

        todo.remove(two);
        QCOMPARE(todo.items().size(), 2);
        todo.remove("no such id");
        QCOMPARE(todo.items().size(), 2);

        todo.toggle(one);
        todo.clearCompleted();
        QCOMPARE(todo.items().size(), 1);
        QCOMPARE(at(todo, 0)["text"].toString(), "three");
    }

    void survivesRestart() {
        QString id;
        {
            TodoService todo;
            todo.add("keep me");
            id = at(todo, 0)["id"].toString();
            todo.toggle(id);
        }
        TodoService restored;
        QCOMPARE(restored.items().size(), 1);
        QCOMPARE(at(restored, 0)["id"].toString(), id);
        QCOMPARE(at(restored, 0)["text"].toString(), "keep me");
        QVERIFY(at(restored, 0)["done"].toBool());
    }

    void corruptFileIsKept() {
        QDir().mkpath(QFileInfo(path()).absolutePath());
        QFile file(path());
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ not json");
        file.close();

        TodoService todo;
        QVERIFY(todo.items().isEmpty());
        todo.add("fresh");

        QFile aside(path() + ".bad");
        QVERIFY(aside.open(QIODevice::ReadOnly));
        QCOMPARE(aside.readAll(), QByteArray("{ not json"));
    }
};

QTEST_GUILESS_MAIN(TodoTest)
#include "todotest.moc"
