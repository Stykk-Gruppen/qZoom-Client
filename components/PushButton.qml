import QtQuick 2.10
import QtQuick.Templates 2.1 as T

T.Button {
    id: control
    height: 30

    //font: Constants.font
    implicitWidth: Math.max(
                       background ? background.implicitWidth : 0,
                       contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(
                        background ? background.implicitHeight : 0,
                        contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: 4
    rightPadding: 4

    text: "My Button"


    background: Rectangle {
        id: buttonBackground
        color: "#5CBB5C"
        implicitWidth: 100
        implicitHeight: 40
        opacity: enabled ? 1 : 0.3
        //border.color: "gray"
        border.width: 0
        radius: 11
    }

    contentItem: Text {
        id: textItem
        text: control.text

        opacity: enabled ? 1.0 : 0.3
        color: "#000000"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: control.clicked()
        onDoubleClicked: control.doubleClicked()
    }

    states: [
        State {
            name: "normal"
            when: !control.down
            PropertyChanges {
                target: buttonBackground
            }
        },
        State {
            name: "down"
            when: control.down
            PropertyChanges {
                target: textItem
                color: "black"
            }
            PropertyChanges {
                target: buttonBackground
                color: "#a2a2a2"
                border.color: "black"
            }
        },
        State {
            name: "red"
            when: control.toggle
            PropertyChanges {
                target: buttonBackground
                color: "#DB504A"
            }
        }

    ]
}
