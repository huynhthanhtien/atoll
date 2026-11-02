// SPDX-FileCopyrightText: 2026 The Atoll contributors
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Atoll

/**
 * Files are being dragged over the island, and where they are let go decides
 * what happens to them. The side under the pointer lights up, since the drop
 * itself is the only click this choice gets.
 */
Item {
    id: view

    property bool canKeep: true
    property bool canShare: true
    /** The side the pointer is over: "keep" or "share". */
    property string choice: "keep"

    implicitWidth: Math.min(Cfg.maxWidth, Math.max(300, tiles.implicitWidth + Math.max(24, Theme.edgeInset * 2)))
    implicitHeight: 84

    component Tile: Rectangle {
        id: tile

        property var iconNames: []
        property string label: ""
        property bool lit: false

        width: 130
        height: 60
        radius: 16
        color: lit ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.28) : Qt.rgba(1, 1, 1, 0.07)
        scale: lit ? 1.05 : 0.97

        Behavior on color {
            ColorAnimation {
                duration: Theme.fast
            }
        }
        Behavior on scale {
            NumberAnimation {
                duration: Theme.normal
                easing.type: Easing.OutBack
            }
        }

        Column {
            anchors.centerIn: parent
            spacing: 5

            IconImage {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 22
                height: 22
                names: tile.iconNames
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: tile.label
                color: Theme.foreground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.size(12)
                font.weight: Font.DemiBold
            }
        }
    }

    Row {
        id: tiles
        anchors.centerIn: parent
        spacing: 10

        Tile {
            visible: view.canKeep
            iconNames: ["folder-download", "document-save", "folder"]
            label: qsTr("Keep here")
            lit: view.choice === "keep" || !view.canShare
        }

        Tile {
            visible: view.canShare
            iconNames: ["document-send", "emblem-shared", "folder-download"]
            label: qsTr("Share")
            lit: view.choice === "share" || !view.canKeep
        }
    }
}
