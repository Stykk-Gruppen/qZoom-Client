import QtQuick 2.0
import QtQuick.Controls 1.4
import "../components" as C

Rectangle {
    color: "dimgray"

    Row {
        id: optionRow
        visible: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32
        Rectangle {
            width: 200
            height: 200
            color: "#141414"
            radius: 10
            Text {
                text: "Join"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    optionRow.visible = false;
                    joinSessionColumn.visible = true
                    backButton.visible = true
                }
            }
        }

        Rectangle {
            width: 200
            height: 200
            color: "#141414"
            radius: 10
            Text {
                text: "Host"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Rectangle {
            width: 200
            height: 200
            color: "#141414"
            radius: 10
            Text {
                text: "Log in"
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Column {
        id: joinSessionColumn
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32
        TextField {
            id: roomIdField
            width: 200
            //height: 32
            text: qsTr("")
            ///font.pixelSize: 24
            placeholderText: qsTr("Room ID")
            selectByMouse: true
            focus: true
        }

        TextField {
            id: roomPasswordField
            width: 200
            //height: 32
            text: qsTr("")
            ///font.pixelSize: 24
            placeholderText: qsTr("Password")
            selectByMouse: true
            focus: true
        }

        C.PushButton {
            id: joinSessionButton
            text: qsTr("Join Session")
            //anchors.verticalCenterOffset: 64
            //visible: false
            font.pixelSize: 32
            onClicked: joinSession()
            onDoubleClicked: changePage("session")
        }

        Text {
            id: errorText
            text: " "
            //visible: true
            font.pixelSize: 18
            color: "red"
        }
    }

    C.PushButton {
        id: backButton
        text: "Back"
        font.pixelSize: 32
        visible: false
        onClicked: {
            joinSessionColumn.visible = false
            optionRow.visible = true
            errorText.text = " "
        }
    }

    function joinSession() {
        if (sessionHandler.joinSession(roomIdField.text, roomPasswordField.text)) {
            changePage("session")
        }
        else {
            console.log("no such session")
            errorText.text = "Failed to join session"
        }
    }

    Component.onCompleted: {
        setTitle("qZoom :: Home");
    }
}
