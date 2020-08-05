import QtQuick 2.0
import QtQuick.Controls 1.4
import "../components" as C

Rectangle {

    //color: "#161637"
    color: "black"
    height: 64
    width: parent.width

    Row {
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32
        anchors.left: parent.left
        layoutDirection: Qt.LeftToRight
        anchors.leftMargin: 32

        Rectangle {
            id: roomInfoRectangle
            color: "#5CBB5C"
            implicitHeight: 30
            implicitWidth: 30
            radius: 11
            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                height: 20
                width: 20
                source: "../img/info-icon.png"
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: roomInfo.open()
            }
        }

        Rectangle {
            id: settingsRectangle
            color: "#5CBB5C"
            implicitHeight: 30
            implicitWidth: 30
            radius: 11
            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                height: 20
                width: 20
                source: "../img/settings-icon.png"
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: showSettings()
            }
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32

        /*
        C.PushButton {
            id: changeAudioInputButton
            text: "Change Audio"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                cb.visible = !cb.visible
            }

            ComboBox {
                id: cb
                width: 200
                visible: false
                model: ListModel {
                    id: audioInputModel
                    function getAudioTypes() {
                        var inputList = streamHandler.getAudioInputDevices()
                        clear()
                        for (var title in inputList) {
                            append({text: inputList[title]})
                        }
                    }
                }
                onActivated: {
                    console.log("switched Audio Input Device to: " + currentText)
                    streamHandler.changeAudioInputDevice(currentText);
                    cb.visible = false
                }
            }
        }
        */



        /*
        C.PushButton {
            id: settingsButton
            text: "Settings"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                showSettings();
            }
        }

        C.PushButton {
            id: roomInfoButton
            text: "Room Info"
            font.pixelSize: 32
            onClicked: roomInfo.open()
        }
        */

        C.PushButton {
            id: muteAudioButton
            text: "Mute Audio"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                if (toggled)
                {
                    state = "red"
                    sessionHandler.disableAudio()
                    toggled = !toggled
                }
                else
                {
                    if(sessionHandler.enableAudio())
                    {
                        state = "normal"
                        toggled = !toggled
                    }
                }
            }
        }

        C.PushButton {
            id: muteVideoButton
            text: "Mute Video"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                if (toggled)
                {
                    state = "red"
                    sessionHandler.disableVideo()
                    toggled = !toggled
                }
                else
                {
                    if(sessionHandler.enableVideo())
                    {
                        shareScreenButton.toggled = false;
                        shareScreenButton.state = "red";
                        state = "normal"
                        toggled = !toggled
                    }
                }
            }
        }

        C.PushButton {
            id: shareScreenButton
            text: "Share Screen"
            font.pixelSize: 32
            property var toggled: false
            onClicked: {
                if (toggled)
                {
                    state = "red"
                    sessionHandler.disableVideo()
                    toggled = !toggled
                }
                else
                {
                    if(sessionHandler.enableScreenShare())
                    {
                        muteVideoButton.toggled = false;
                        muteVideoButton.state = "red";

                        state = "normal"
                        toggled = !toggled
                    }
                }
            }
        }

        /* C.PushButton {
            id: addScreenButton
            text: "Add Screens"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                repeaterId.model = 0;
                gridId.columns = gridId.calcColumns();
                gridId.rows = gridId.calcRows();
                repeaterId.model = imageHandler.getNumberOfScreens();
            }
        }*/


        //TODO maybe this function does not need to be here?
        //Creates a Signal connection to imageHandler,
        //when the signal addScreen is emitted, this function will run
        Connections {
            target: imageHandler
            function onRefreshScreens() {
                repeaterId.model = 0;
                gridId.columns = gridId.calcColumns();
                gridId.rows = gridId.calcRows();
                repeaterId.model = imageHandler.getNumberOfScreens();
            }
        }


    }

    Row {
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32
        anchors.right: parent.right
        layoutDirection: Qt.RightToLeft
        anchors.rightMargin: 32

        C.PushButton {
            id: leaveSessionButton
            text: "Leave Session"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                //streamHandler.stopRecording();
                //TODO handle return value?
                sessionHandler.leaveSession();
                changePage("home");
            }
        }
    }

    Component.onCompleted: {
        //audioInputModel.getAudioTypes()
        shareScreenButton.state = "red";
        if(!sessionHandler.checkAudioEnabled())
        {
            muteAudioButton.toggled = false;
            muteAudioButton.state = "red";
        }
        if(!sessionHandler.checkVideoEnabled())
        {
            muteVideoButton.toggled = false;
            muteVideoButton.state = "red";
        }
    }
}
