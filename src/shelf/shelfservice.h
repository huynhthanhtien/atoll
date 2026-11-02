// SPDX-FileCopyrightText: 2026 The Atoll contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

/**
 * Files parked on the island: dropped here from one window, dragged out into
 * another a moment later.
 *
 * Only the addresses are held. Nothing is copied, nothing touches the disk and
 * the list does not outlive the process, so a file on the shelf is exactly the
 * file it was and the shelf is never one more place something has to be deleted
 * from.
 */
class ShelfService : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Accessed through App.shelf")

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /** The icon that stands for the whole shelf on the resting island. */
    Q_PROPERTY(QStringList leadIcon READ leadIcon NOTIFY countChanged)
    /** A drag out of the shelf is in progress. */
    Q_PROPERTY(bool dragging READ dragging NOTIFY draggingChanged)

public:
    enum Role { NameRole = Qt::UserRole + 1, UrlRole, IconRole };

    explicit ShelfService(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const
    {
        return m_items.size();
    }
    QStringList leadIcon() const;
    bool dragging() const
    {
        return m_dragging;
    }

    /** Whatever is not a file that exists is left out; what is already here stays once. */
    Q_INVOKABLE void add(const QList<QUrl> &urls);
    /** The same, for callers that only have paths (D-Bus, the CLI). */
    Q_INVOKABLE void addPaths(const QStringList &paths);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void clear();
    /** Let go of files that were moved or deleted since they were dropped. */
    Q_INVOKABLE void prune();
    /** Start dragging one file, or all of them with a row of -1. */
    Q_INVOKABLE void drag(int row = -1);

Q_SIGNALS:
    void countChanged();
    void draggingChanged();

private:
    struct Item {
        QUrl url;
        QString name;
        QStringList icon;
    };

    void removeUrls(const QList<QUrl> &urls);

    QList<Item> m_items;
    bool m_dragging = false;
};
