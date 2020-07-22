import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C



Dialog {

    function loadSettings() {
        defaultAudioInput.model = streamHandler.getAudioInputDevices();
        console.log(backendSettings.getDefaultAudioInput());
        var index = defaultAudioInput.find(backendSettings.getDefaultAudioInput())
        console.log(index);
        defaultAudioInput.currentIndex = index;
        audioOn.checked = backendSettings.getAudioOn();
        videoOn.checked = backendSettings.getVideoOn();
        displayName.text = backendSettings.getDisplayName();

    }

    /*function getAudioInputDevices(){
        var inputList = streamHandler.getAudioInputDevices();
        for(var title in inputList) {
            inputAudioDevices.append({text: inputList[title]});
        }
        defaultAudioInput.displayText = streamHandler.getDefaultAudioInputDevice();

    }*/

    id: settings
    visible: false
    width: 400
    height: 300
    //modal: true
    title: "Settings"
    //anchors.centerIn: parent
    standardButtons: Dialog.Apply | Dialog.Close | Dialog.RestoreDefaults


    Rectangle{

        ColumnLayout {
            RowLayout {
                Text {
                    text: qsTr("Visible name")
                }
                TextField {
                    id: displayName
                    placeholderText: "UserName"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            CheckBox {
                id: audioOn
                checked: true
                text: qsTr("Audio on when joining meeting")
            }
            CheckBox {
                id: videoOn
                checked: true
                text: qsTr("Video on when joining meeting")
            }

            RowLayout {

                Text {
                    text: qsTr("Default input device");
                }

                ComboBox {
                    id: defaultAudioInput
                    editable: true


                }
            }
        }
    }
    onApply: {
        console.log("Apply clicked")
        backendSettings.setAudioOn(audioOn.checked);
        backendSettings.setVideoOn(videoOn.checked);
        backendSettings.setDisplayName(displayName.text === ""? "UserName" : displayName.text);

            backendSettings.setDisplayName(displayName.text);
        backendSettings.setDefaultAudioInput(defaultAudioInput.currentText);
        backendSettings.saveSettings();
    }
    onRejected: console.log("Close clicked")
    onReset: {
        console.log("Restore Defaults clicked")
        backendSettings.loadAndSaveDefaultSettings();
        loadSettings();
    }
}

