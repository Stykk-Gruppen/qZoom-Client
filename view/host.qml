import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Window 2.12
import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C


Rectangle {
    //color: "#161637"
    id: hostWindow
    color: Qt.rgba(27/255, 29/255, 54/255, 1)



    Column {

        spacing: 10
        anchors.fill: parent
        anchors.top: parent.top
        C.Header {
            id: header
            color: hostWindow.color
            //anchors.top: parent.top
        }

        C.PushButton {
            id: backButton
            text: "Back"
            font.pixelSize: 32
            visible: true
            onClicked: changePage("home")()
        }
    }


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


