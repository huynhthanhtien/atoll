// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

/**
 * The to-do list behind the dashboard's Todo tab. Every change goes straight to
 * disk, so nothing depends on the island shutting down cleanly.
 */
class TodoService : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Accessed through App.todo")
    Q_PROPERTY(QVariantList items READ items NOTIFY changed)
    Q_PROPERTY(int remaining READ remaining NOTIFY changed)
public:
    explicit TodoService(QObject *parent = nullptr);

    QVariantList items() const;
    int remaining() const;

    /** Whitespace is tidied up first; returns false if nothing was left to add. */
    Q_INVOKABLE bool add(const QString &text);
    Q_INVOKABLE void toggle(const QString &id);
    Q_INVOKABLE void remove(const QString &id);
    Q_INVOKABLE void clearCompleted();

Q_SIGNALS:
    void changed();

private:
    struct Item {
        QString id;
        QString text;
        bool done = false;
    };

    void load();
    void commit();

    QList<Item> m_items;
    QString m_path;
};
