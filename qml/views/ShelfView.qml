// SPDX-FileCopyrightText: 2026 The Atoll contributors
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import Atoll

/**
 * The files parked on the island, opened by resting the pointer on it. Each one
 * is dragged out on its own, or all of them at once from the first chip; a file
 * that lands somewhere leaves the shelf.
 */
Item {
    id: view

    readonly property real inset: Math.max(14, Theme.edgeInset)
    readonly property bool several: App.shelf.count > 1
    // What the row may take before the list has to scroll: everything that is
    // not the list, out of the widest the island goes.
    readonly property real listRoom: Cfg.maxWidth - inset * 2 - clearButton.width - row.spacing
                                     - (several ? allChip.width + row.spacing : 0)

    implicitWidth: Math.min(Cfg.maxWidth, Math.max(220, row.implicitWidth + inset * 2))
    implicitHeight: 58

    Component.onCompleted: App.shelf.prune()

    component Chip: Rectangle {
        id: chip

        property var iconNames: []
        property string label: ""
        /** The shelf row this chip drags, or -1 for all of them. */
        property int shelfRow: -1
        property bool removable: true
        property bool lifted: false

        signal removeRequested()

        implicitWidth: chipContent.implicitWidth + 22
        width: implicitWidth
        height: 34
        radius: 12
        color: grab.containsMouse || closeArea.containsMouse ? Qt.rgba(1, 1, 1, 0.16) : Qt.rgba(1, 1, 1, 0.08)
        opacity: lifted && App.shelf.dragging ? 0.45 : 1
        scale: grab.pressed ? 0.96 : 1

        Behavior on color {
            ColorAnimation {
                duration: Theme.fast
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: Theme.fast
            }
        }
        Behavior on scale {
            NumberAnimation {
                duration: Theme.fast
            }
        }

        Connections {
            target: App.shelf
            function onDraggingChanged() {
                if (!App.shelf.dragging) {
                    chip.lifted = false
                }
            }
        }

        Row {
            id: chipContent
            anchors.verticalCenter: parent.verticalCenter
            x: 11
            spacing: 6

            IconImage {
                anchors.verticalCenter: parent.verticalCenter
                width: 18
                height: 18
                names: chip.iconNames
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(implicitWidth, 130)
                text: chip.label
                color: Theme.foreground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.size(11)
                font.weight: Font.Medium
                elide: Text.ElideMiddle
            }
        }

        MouseArea {
            id: grab

            property point origin

            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

            onPressed: mouse => origin = Qt.point(mouse.x, mouse.y)
            onPositionChanged: mouse => {
                if (!pressed || App.shelf.dragging) {
                    return
                }
                const distance = Math.hypot(mouse.x - origin.x, mouse.y - origin.y)
                if (distance >= Qt.styleHints.startDragDistance) {
                    chip.lifted = true
                    App.shelf.drag(chip.shelfRow)
                }
            }
        }

        // Sits over the corner of the chip and only shows when it is wanted, so
        // the row does not have to make room for a button per file.
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 2
            width: 14
            height: 14
            radius: 7
            visible: chip.removable
            color: closeArea.containsMouse ? Theme.critical : Qt.rgba(0, 0, 0, 0.75)
            opacity: grab.containsMouse || closeArea.containsMouse ? 1 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.fast
                }
            }

            Text {
                anchors.centerIn: parent
                text: "×"
                color: Theme.foreground
                font.pixelSize: 11
            }

            MouseArea {
                id: closeArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: chip.removeRequested()
            }
        }
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 7

        Chip {
            id: allChip
            visible: view.several
            iconNames: ["folder-documents", "folder"]
            label: qsTr("All %1").arg(App.shelf.count)
            removable: false
        }

        ListView {
            id: list

            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(0, Math.min(contentWidth, view.listRoom))
            height: 40
            orientation: ListView.Horizontal
            spacing: 7
            clip: true
            // The pointer on this row is for picking files up, not for
            // flicking; the wheel below does the scrolling.
            interactive: false
            model: App.shelf

            delegate: Chip {
                required property int index
                required property string name
                required property var icon
                y: (list.height - height) / 2
                iconNames: icon
                label: name
                shelfRow: index
                onRemoveRequested: App.shelf.remove(index)
            }

            add: Transition {
                NumberAnimation {
                    property: "scale"
                    from: 0.6
                    to: 1
                    duration: Theme.normal
                    easing.type: Easing.OutBack
                }
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: Theme.fast
                }
            }
            remove: Transition {
                NumberAnimation {
                    property: "scale"
                    to: 0.6
                    duration: Theme.fast
                }
                NumberAnimation {
                    property: "opacity"
                    to: 0
                    duration: Theme.fast
                }
            }
            displaced: Transition {
                NumberAnimation {
                    property: "x"
                    duration: Theme.normal
                    easing.type: Easing.OutCubic
                }
            }

            WheelHandler {
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                onWheel: event => {
                    const delta = event.angleDelta.x !== 0 ? event.angleDelta.x : event.angleDelta.y
                    const furthest = Math.max(0, list.contentWidth - list.width)
                    list.contentX = Math.max(0, Math.min(furthest, list.contentX - delta / 2))
                }
            }
        }

        PillButton {
            id: clearButton
            anchors.verticalCenter: parent.verticalCenter
            implicitWidth: 30
            implicitHeight: 30
            label: "×"
            onClicked: App.shelf.clear()
        }
    }
}
