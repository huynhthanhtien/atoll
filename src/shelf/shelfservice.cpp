// SPDX-FileCopyrightText: 2026 The Atoll contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "shelfservice.h"

#include <QDrag>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QMimeDatabase>
#include <QTimer>

using namespace Qt::StringLiterals;

ShelfService::ShelfService(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ShelfService::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant ShelfService::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }
    const Item &item = m_items.at(index.row());
    switch (role) {
    case NameRole:
        return item.name;
    case UrlRole:
        return item.url;
    case IconRole:
        return item.icon;
    default:
        return {};
    }
}

QHash<int, QByteArray> ShelfService::roleNames() const
{
    return {{NameRole, "name"}, {UrlRole, "url"}, {IconRole, "icon"}};
}

QStringList ShelfService::leadIcon() const
{
    return m_items.isEmpty() ? QStringList{} : m_items.constLast().icon;
}

void ShelfService::add(const QList<QUrl> &urls)
{
    static const QMimeDatabase mimeDatabase;

    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QFileInfo info(url.toLocalFile());
        const bool known = std::any_of(m_items.cbegin(), m_items.cend(), [&url](const Item &item) {
            return item.url == url;
        });
        if (known || !info.exists()) {
            continue;
        }
        const QMimeType type = mimeDatabase.mimeTypeForFile(info);
        beginInsertRows({}, m_items.size(), m_items.size());
        m_items.append({url, info.fileName(), {type.iconName(), type.genericIconName(), u"text-x-generic"_s}});
        endInsertRows();
        Q_EMIT countChanged();
    }
}

void ShelfService::addPaths(const QStringList &paths)
{
    QList<QUrl> urls;
    urls.reserve(paths.size());
    for (const QString &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    add(urls);
}

void ShelfService::remove(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }
    beginRemoveRows({}, row, row);
    m_items.removeAt(row);
    endRemoveRows();
    Q_EMIT countChanged();
}

void ShelfService::clear()
{
    if (m_items.isEmpty()) {
        return;
    }
    beginResetModel();
    m_items.clear();
    endResetModel();
    Q_EMIT countChanged();
}

void ShelfService::prune()
{
    for (int row = m_items.size() - 1; row >= 0; --row) {
        if (!QFileInfo::exists(m_items.at(row).url.toLocalFile())) {
            remove(row);
        }
    }
}

void ShelfService::removeUrls(const QList<QUrl> &urls)
{
    for (int row = m_items.size() - 1; row >= 0; --row) {
        if (urls.contains(m_items.at(row).url)) {
            remove(row);
        }
    }
}

void ShelfService::drag(int row)
{
    if (m_dragging || row < -1 || row >= m_items.size()) {
        return;
    }
    prune();
    if (m_items.isEmpty() || row >= m_items.size()) {
        return;
    }

    QList<QUrl> urls;
    if (row >= 0) {
        urls.append(m_items.at(row).url);
    } else {
        for (const Item &item : std::as_const(m_items)) {
            urls.append(item.url);
        }
    }
    const QStringList icons = row >= 0 ? m_items.at(row).icon : leadIcon();

    m_dragging = true;
    Q_EMIT draggingChanged();

    // exec() spins its own event loop until the drop, which must not happen
    // inside the pointer event that started the drag: the handler it came from
    // is still on the stack, and the view holding it may well go away.
    QTimer::singleShot(0, this, [this, urls, icons] {
        auto *mime = new QMimeData;
        mime->setUrls(urls);

        auto *drag = new QDrag(this);
        drag->setMimeData(mime);
        for (const QString &name : icons) {
            const QIcon icon = QIcon::fromTheme(name);
            if (!icon.isNull()) {
                drag->setPixmap(icon.pixmap(48));
                break;
            }
        }

        // Copy only: a file manager told "move" would take the file away from
        // wherever it was, which nobody asked for by parking it here.
        const Qt::DropAction result = drag->exec(Qt::CopyAction);

        m_dragging = false;
        Q_EMIT draggingChanged();
        if (result != Qt::IgnoreAction) {
            removeUrls(urls);
        }
    });
}
