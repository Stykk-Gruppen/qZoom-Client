import QtQuick 2.0
import QtQuick.Controls 1.4
import "../components" as C

Rectangle {
    color: "#141414"
    height: 64
    width: parent.width

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32

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
                    id: t
                    function getAudioTypes() {
                        var inputList = streamHandler.getAudioInputDevices()
                        clear()
                        for (var title in inputList) {
                            append({text: inputList[title]})
                        }
                    }
                }
                onActivated: {
                    console.log("switched Audio Input Device to: " + currentValue)
                    streamHandler.changeAudioInputDevice(currentValue);
                    cb.visible = false
                }
            }

        }

        C.PushButton {
            id: muteAudioButton
            text: "Mute Audio"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                if (toggled) {
                    state = "red"
                    streamHandler.disableAudio()
                } else {
                    state = "normal"
                    streamHandler.enableVideo()
                }
                toggled = !toggled
            }
        }

        C.PushButton {
            id: muteVideoButton
            text: "Mute Video"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                if (toggled) {
                    state = "red"
                    streamHandler.disableVideo()
                } else {
                    state = "normal"
                    streamHandler.enableVideo()
                }
                toggled = !toggled
            }
        }

        C.PushButton {
            id: addScreenButton
            text: "Add Screens"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                //addScreen();
                repeaterId.model = 0;
                gridId.columns = 3;
                gridId.rows = 2;
                repeaterId.model = 5;
            }
        }

        C.PushButton {
            id: leaveSessionButton
            text: "Leave Session"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                streamHandler.stopRecording();
                changePage("home");
            }
        }
    }

    Component.onCompleted: {
        t.getAudioTypes()
    }
}
