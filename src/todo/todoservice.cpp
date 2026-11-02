// SPDX-License-Identifier: GPL-3.0-or-later
#include "todoservice.h"
#include <algorithm>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <QVariantMap>

TodoService::TodoService(QObject *parent) : QObject(parent) {
    m_path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/todos.json";
    load();
}

void TodoService::load() {
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    if (error.error != QJsonParseError::NoError) {
        // The next save would overwrite the list for good, so keep what is there.
        const QString aside = m_path + ".bad";
        QFile::remove(aside);
        QFile::rename(m_path, aside);
        qWarning("atoll: unreadable to-do list moved to %s: %s", qUtf8Printable(aside),
                 qUtf8Printable(error.errorString()));
        return;
    }
    const QJsonArray items = document.object().value("items").toArray();
    for (const QJsonValue &value : items) {
        const QJsonObject object = value.toObject();
        const QString text = object.value("text").toString();
        if (text.isEmpty()) {
            continue;
        }
        QString id = object.value("id").toString();
        if (id.isEmpty()) {
            id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        m_items.append({id, text, object.value("done").toBool()});
    }
}

void TodoService::commit() {
    QJsonArray items;
    for (const Item &item : std::as_const(m_items)) {
        items.append(QJsonObject{{"id", item.id}, {"text", item.text}, {"done", item.done}});
    }
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)
        || file.write(QJsonDocument(QJsonObject{{"items", items}}).toJson()) < 0
        || !file.commit()) {
        qWarning("atoll: could not save the to-do list to %s: %s", qUtf8Printable(m_path),
                 qUtf8Printable(file.errorString()));
    }
    Q_EMIT changed();
}

QVariantList TodoService::items() const {
    QVariantList list;
    list.reserve(m_items.size());
    for (const Item &item : m_items) {
        list.append(QVariantMap{{"id", item.id}, {"text", item.text}, {"done", item.done}});
    }
    return list;
}

int TodoService::remaining() const {
    return std::count_if(m_items.cbegin(), m_items.cend(), [](const Item &item) { return !item.done; });
}

bool TodoService::add(const QString &text) {
    const QString tidy = text.simplified();
    if (tidy.isEmpty()) {
        return false;
    }
    m_items.append({QUuid::createUuid().toString(QUuid::WithoutBraces), tidy, false});
    commit();
    return true;
}

void TodoService::toggle(const QString &id) {
    for (Item &item : m_items) {
        if (item.id == id) {
            item.done = !item.done;
            commit();
            return;
        }
    }
}

void TodoService::remove(const QString &id) {
    if (m_items.removeIf([&id](const Item &item) { return item.id == id; }) > 0) {
        commit();
    }
}

void TodoService::clearCompleted() {
    if (m_items.removeIf([](const Item &item) { return item.done; }) > 0) {
        commit();
    }
}
