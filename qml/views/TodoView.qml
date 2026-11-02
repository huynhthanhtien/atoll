// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls.Basic
import Atoll

/**
 * The dashboard's Todo tab. The rows lay out into the page, as the notification
 * backlog does, so a long list scrolls with the dashboard around it.
 */
Column {
    id: root

    /** The surface to ask for the keyboard: a layer surface has none by default. */
    property var keyboardWindow: null

    readonly property var items: App.todo.items
    readonly property int doneCount: items.length - App.todo.remaining

    function requestKeyboard(wanted) {
        if (keyboardWindow && !App.ai.engaged && !App.office.open) {
            App.shell.setKeyboardFocus(keyboardWindow, wanted)
        }
    }

    spacing: 6

    Component.onDestruction: requestKeyboard(false)

    TextField {
        id: input
        // Island.qml keeps the dashboard open while this holds the focus.
        objectName: "todoInput"
        width: parent.width
        height: 36
        leftPadding: 12
        rightPadding: 12
        maximumLength: 300
        placeholderText: qsTr("Add a task…")
        color: Theme.foreground
        placeholderTextColor: Theme.muted
        font.family: Theme.fontFamily
        font.pixelSize: Theme.size(12)
        background: Rectangle {
            radius: 12
            color: Theme.tint(input.activeFocus ? 0.1 : 0.06)
        }

        onActiveFocusChanged: root.requestKeyboard(activeFocus)
        // Stays in the field, so several tasks can be typed in a row.
        onAccepted: if (App.todo.add(text)) {
            clear()
        }
        Keys.onEscapePressed: focus = false

        // A MouseArea rather than a TapHandler, which would let the click through
        // to the island's own and fold the dashboard. Once the field has the
        // focus this steps aside and leaves the pointer to the text.
        MouseArea {
            anchors.fill: parent
            enabled: !input.activeFocus
            cursorShape: Qt.IBeamCursor
            onClicked: {
                root.requestKeyboard(true)
                input.forceActiveFocus()
            }
        }
    }

    Item {
        visible: root.items.length === 0
        width: parent.width
        height: 72

        Text {
            anchors.centerIn: parent
            text: qsTr("Nothing to do")
            color: Theme.muted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.size(13)
        }
    }

    Repeater {
        model: root.items

        delegate: Rectangle {
            id: row
            required property var modelData

            readonly property bool done: modelData.done

            width: root.width
            height: Math.max(36, label.implicitHeight + 16)
            radius: 12
            color: Theme.tint(hover.hovered ? 0.1 : 0.05)

            Behavior on color {
                ColorAnimation {
                    duration: Theme.fast
                }
            }

            HoverHandler {
                id: hover
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: App.todo.toggle(row.modelData.id)
            }

            Rectangle {
                id: tick
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                width: 16
                height: 16
                radius: 8
                color: row.done ? Theme.accent : "transparent"
                border.width: row.done ? 0 : 1.5
                border.color: Theme.muted
            }

            Text {
                id: label
                anchors.left: tick.right
                anchors.leftMargin: 10
                anchors.right: remove.left
                anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                text: row.modelData.text
                // Typed by the user, so never to be read as markup.
                textFormat: Text.PlainText
                color: row.done ? Theme.muted : Theme.foreground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.size(12)
                font.strikeout: row.done
                wrapMode: Text.Wrap
                maximumLineCount: 3
                elide: Text.ElideRight
            }

            RoundButton {
                id: remove
                anchors.right: parent.right
                anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                width: 24
                height: 24
                opacity: hover.hovered ? 1 : 0
                icon: ["window-close"]
                onClicked: App.todo.remove(row.modelData.id)
            }
        }
    }

    Item {
        visible: root.items.length > 0
        width: parent.width
        height: 30

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("%1 left").arg(App.todo.remaining)
            color: Theme.muted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.size(11)
        }

        PillButton {
            visible: root.doneCount > 0
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            implicitHeight: 26
            label: qsTr("Clear completed")
            onClicked: App.todo.clearCompleted()
        }
    }
}
