import QtQuick 2.0
import QtQuick.Controls 1.4
import "../components" as C

Rectangle {
    color: "dimgray"

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 96
        Column {
            spacing: 32
            C.PushButton {
                id: joinRoomButton
                text: qsTr("Join Room")
                font.pixelSize: 32
                onClicked: joinSession()
            }
        }

        Column {
            spacing: 32
            TextField {
                id: roomIdField
                width: 200
                text: roomIdField.text = userHandler.getPersonalRoomId()
                placeholderText: qsTr("Room ID")
                selectByMouse: true
                focus: true
            }

            TextField {
                id: roomPasswordField
                width: 200
                text: userHandler.getPersonalRoomPassword()
                placeholderText: qsTr("Password")
                selectByMouse: true
                focus: true
            }

            C.PushButton {
                id: changeRoomValuesButton
                text: qsTr("Update")
                font.pixelSize: 32
                onClicked: changeRoomValues()
            }

            Text {
                id: errorText
                text: " "
                font.pixelSize: 18
            }
        }
    }

    C.PushButton {
        id: backButton
        text: "Back"
        font.pixelSize: 32
        onClicked: changePage("home")
    }

    function changeRoomValues() {
        if (userHandler.updatePersonalRoom(roomIdField.text, roomPasswordField.text)) {
            errorText.text = "Successfully updated";
            errorText.color = "lime";
        }
        else {
            errorText.text = "Failed to update room";
            errorText.color = "red";
        }
    }

    function joinSession() {
        if (sessionHandler.joinSession(userHandler.getPersonalRoomId(), userHandler.getPersonalRoomPassword())) {
            changePage("session")
        }
        else {
            console.log("no such session")
            errorText.text = "Failed to join session"
        }
    }


    Component.onCompleted: {
        setTitle("qZoom :: Host")
    }
}


