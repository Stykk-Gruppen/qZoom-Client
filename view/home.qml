import QtQuick 2.0
import QtQuick.Controls 1.4
import "../components" as C

Rectangle {
    //color: "#161637"
    id: root
    color: Qt.rgba(27/255, 29/255, 54/255, 1)

    /*
    C.Header {
        id: header
        anchors.top: parent.top
    }
    */

    Column {

        spacing: 10
        anchors.fill: parent
        anchors.top: parent.top
        C.Header {
            id: header
            color: root.color
            //anchors.top: parent.top
        }

        C.PushButton {
            id: backButton
            text: "Back"
            font.pixelSize: 32
            visible: false
            onClicked: clear()
        }

        /*
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            Rectangle {
                id: joinRectangle2
                width: 200
                height: 200
                color: "#5CBB5C"
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
                        joinSessionButton.visible = true

                        if (userHandler.isGuest()) {
                            displayNameField.text = backendSettings.getDisplayName();
                            displayNameField.visible = true;
                        } else {
                            displayNameField.visible = false;
                        }

                        if (backendSettings.getSaveLastRoom()) {
                            roomIdField.text = backendSettings.getLastRoomId();
                            roomPasswordField.text = backendSettings.getLastRoomPassword();
                        }
                    }
                }
            }
        }
        */
    }

        Row {
            id: optionRow
            visible: true
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: 32


            Column {
                spacing: 15
                Rectangle {
                    id: joinSessionRectangle
                    width: 200
                    height: 200
                    color: "#5CBB5C"
                    radius: 24

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width: 125
                        height: 125
                        source: "../img/join-session-icon.png"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            optionRow.visible = false;
                            joinSessionColumn.visible = true
                            backButton.visible = true
                            joinSessionButton.visible = true

                            if (userHandler.isGuest()) {
                                displayNameField.text = backendSettings.getDisplayName();
                                displayNameField.visible = true;
                            } else {
                                displayNameField.visible = false;
                            }

                            if (backendSettings.getSaveLastRoom()) {
                                roomIdField.text = backendSettings.getLastRoomId();
                                roomPasswordField.text = backendSettings.getLastRoomPassword();
                            }
                        }
                    }
                }

                Text {
                    text: "Join Session"
                    color: "white"
                    font.pixelSize: 24
                    anchors.horizontalCenter: joinSessionRectangle.horizontalCenter
                    //anchors.verticalCenter: parent.verticalCenter
                }
            }

            /*
            Rectangle {
                id: joinRectangle
                width: 200
                height: 200
                color: "#5CBB5C"
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
                        joinSessionButton.visible = true

                        if (userHandler.isGuest()) {
                            displayNameField.text = backendSettings.getDisplayName();
                            displayNameField.visible = true;
                        } else {
                            displayNameField.visible = false;
                        }

                        if (backendSettings.getSaveLastRoom()) {
                            roomIdField.text = backendSettings.getLastRoomId();
                            roomPasswordField.text = backendSettings.getLastRoomPassword();
                        }
                    }
                }
            }
            */


            Column {
                id: hostSessionColumn
                visible: !userHandler.isGuest()
                spacing: 15
                Rectangle {
                    id: hostSessionRectangle
                    width: 200
                    height: 200
                    color: "#5CBB5C"
                    radius: 24

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        source: "../img/host-session-icon.png"
                        height: 125
                        width: 125
                    }

                    MouseArea {
                        id: hostMouseArea
                        anchors.fill: parent
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        enabled: !userHandler.isGuest()
                        onClicked: {
                            if (userHandler.hasRoom()) {
                                changePage("host")
                            }
                            else {
                                optionRow.visible = false;
                                joinSessionColumn.visible = true
                                backButton.visible = true
                                createSessionButton.visible = true
                            }
                        }
                    }
                }

                Text {
                    text: "Host Session"
                    color: "white"
                    font.pixelSize: 24
                    anchors.horizontalCenter: hostSessionRectangle.horizontalCenter
                    //anchors.verticalCenter: parent.verticalCenter
                }
            }

            Column {
                id: logInColumn
                visible: userHandler.isGuest()
                spacing: 15
                Rectangle {
                    id: logInRectangle
                    width: 200
                    height: 200
                    color: "#5CBB5C"
                    radius: 24

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        source: "../img/login-icon.png"
                        height: 125
                        width: 125
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            optionRow.visible = false;
                            loginColumn.visible = true
                            backButton.visible = true
                        }
                    }
                }

                Text {
                    text: "Log In"
                    color: "white"
                    font.pixelSize: 24
                    anchors.horizontalCenter: logInRectangle.horizontalCenter
                    //anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

            /*
            Rectangle {
                id: hostRectangle
                width: 200
                height: 200
                color: "#5CBB5C"
                radius: 10
                Text {
                    text: "Host"
                    color: "white"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
                MouseArea {
                    id: hostMouseArea
                    anchors.fill: parent
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    enabled: !userHandler.isGuest()
                    onClicked: {
                        if (userHandler.hasRoom()) {
                            changePage("host")
                        }
                        else {
                            optionRow.visible = false;
                            joinSessionColumn.visible = true
                            backButton.visible = true
                            createSessionButton.visible = true
                        }
                    }
                }
            }
            */
            /*
            Rectangle {
                id: logInRectangle
                width: 200
                height: 200
                color: "#5CBB5C"
                radius: 24
                Text {
                    text: "Log in"
                    color: "white"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        optionRow.visible = false;
                        loginColumn.visible = true
                        backButton.visible = true
                    }
                }
            }
            */

            /*
            Rectangle {
                id: settingsRectangle
                width: 200
                height: 200
                color: "#5CBB5C"
                radius: 10
                Text {
                    text: "Settings"
                    color: "white"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        showSettings();
                    }
                }
            }
        }
        */





    Column {
        id: joinSessionColumn
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32

        TextField {
            id: displayNameField
            width: 200
            //height: 32
            text: qsTr("")
            ///font.pixelSize: 24
            placeholderText: qsTr("Display Name")
            selectByMouse: true
            focus: true
            visible: false
        }
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
            visible: false
            font.pixelSize: 32
            onClicked: joinSession()
            onDoubleClicked: {
                if (sessionHandler.joinSession("Debug", "123")) {
                    changePage("session")
                }
                else {
                    console.log("no such session")
                    changePage("session")
                }
            }
        }

        C.PushButton {
            id: createSessionButton
            text: qsTr("Create Session")
            visible: false
            font.pixelSize: 32
            onClicked: createSession()
        }

        Text {
            id: errorText
            text: " "
            //visible: true
            font.pixelSize: 18
            color: "red"
        }
    }

    Column {
        id: loginColumn
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32
        TextField {
            id: loginUsernameField
            width: 200
            //height: 32
            text: qsTr("")
            ///font.pixelSize: 24
            placeholderText: qsTr("Username")
            selectByMouse: true
            focus: true
        }

        TextField {
            id: loginPasswordField
            width: 200
            //height: 32
            text: qsTr("")
            ///font.pixelSize: 24
            placeholderText: qsTr("Password")
            //echoMode: "Password"
            selectByMouse: true
            focus: true
        }

        C.PushButton {
            id: loginButton
            text: qsTr("Login")
            //anchors.verticalCenterOffset: 64
            //visible: false
            font.pixelSize: 32
            onClicked: login()
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

    function login() {
        if (userHandler.login(loginUsernameField.text, loginPasswordField.text)) {
            console.log("Successfully logged in!")
            clear()
            hostSessionColumn.visible = true
            logInColumn.visible = false
            hostMouseArea.enabled = true;
            hostMouseArea.cursorShape = Qt.PointingHandCursor

        }
        else {
            console.log("Failed to login")
            errorText.text = "Failed to login"
        }
    }

    function createSession() {
        if (sessionHandler.createSession(roomIdField.text, roomPasswordField.text)) {
            console.log("Successfully created session.")
            changePage("session")
        }
        else {
            console.log("Failed to create session")
            errorText.text = "Failed to create session"
        }
    }

    function clear() {
        joinSessionColumn.visible = false
        loginColumn.visible = false
        optionRow.visible = true
        errorText.text = " "
        backButton.visible = false
        joinSessionButton.visible = false
        createSessionButton.visible = false
    }

    Component.onCompleted: {
        setTitle("qZoom :: Home");
    }
}
