// SPDX-FileCopyrightText: 2026 The Atoll contributors
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Atoll

/**
 * What the resting island says about files parked on it: an icon and how many.
 * A count and nothing else, because file names are not for the lock screen.
 */
Row {
    id: badge

    readonly property int count: App.shelf.count

    visible: count > 0 && (Cfg.modules.shelf ?? true) && !App.lock.locked
    spacing: 4

    // Buds out of the island as it shrinks around it, and again per new file.
    function pop() {
        popIn.restart()
    }
    onCountChanged: pop()
    Component.onCompleted: pop()

    SequentialAnimation {
        id: popIn
        NumberAnimation {
            target: badge
            property: "scale"
            from: 0.55
            to: 1
            duration: Theme.slow
            easing.type: Easing.OutBack
            easing.overshoot: 2.2
        }
    }

    IconImage {
        anchors.verticalCenter: parent.verticalCenter
        width: 14
        height: 14
        names: App.shelf.leadIcon
    }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        text: badge.count
        color: Theme.foreground
        font.family: Theme.fontFamily
        font.pixelSize: Theme.size(11)
        font.weight: Font.DemiBold
    }
}
