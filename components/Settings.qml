import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C



Dialog {

    function loadSettings() {
        defaultAudioInput.model = sessionHandler.getAudioInputDevices();
        var index = defaultAudioInput.find(backendSettings.getDefaultAudioInput())
        var sessionIsActive = sessionHandler.getSessionIsActive();
        defaultAudioInput.currentIndex = index;
        audioOn.checked = backendSettings.getAudioOn();
        videoOn.checked = backendSettings.getVideoOn();
        saveLastRoom.checked = backendSettings.getSaveLastRoom();
        displayName.text = backendSettings.getDisplayName();
        hostAddressTextField.text = backendSettings.getServerIpAddress();
        hostAddressTextField.readOnly = sessionIsActive;
        tcpPortTextField.text = backendSettings.getTcpPort();
        tcpPortTextField.readOnly = sessionHandler;
        udpPortTextField.text = backendSettings.getUdpPort();
        udpPortTextField.readOnly = sessionIsActive;
        sqlTcpPortTextField.text = backendSettings.getSqlTcpPort();
        sqlTcpPortTextField.readOnly = sessionIsActive;
    }

    id: settings
    visible: false
    width: 400
    height: 500
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
                    placeholderText: "Visible name"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    text: qsTr("Host Address");
                }
                TextField {
                    id: hostAddressTextField
                    placeholderText: "Localhost"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    text: qsTr("UDP Port");
                }
                TextField {
                    id: udpPortTextField
                    placeholderText: "1337"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    text: qsTr("TCP Port");
                }
                TextField {
                    id: tcpPortTextField
                    placeholderText: "1338"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    text: qsTr("SQL TCP Port");
                }
                TextField {
                    id: sqlTcpPortTextField
                    placeholderText: "1339"
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

            CheckBox {
                id: saveLastRoom
                checked: true
                text: qsTr("Remember last room")
            }

            RowLayout {
                Text {
                    text: qsTr("Default input device");
                }

                ComboBox {
                    id: defaultAudioInput
                    editable: false
                }
            }
        }
    }
    onApply: {
        console.log("Apply clicked")

        backendSettings.setAudioOn(audioOn.checked);
        backendSettings.setVideoOn(videoOn.checked);
        backendSettings.setSaveLastRoom(saveLastRoom.checked);
        backendSettings.setDefaultAudioInput(defaultAudioInput.currentText);
        backendSettings.setServerIpAddress(hostAddressTextField.text);
        backendSettings.setTcpPort(tcpPortTextField.text);
        backendSettings.setUdpPort(udpPortTextField.text);
        backendSettings.setSqlTcpPort(sqlTcpPortTextField.text);

        if (displayName.text !== backendSettings.getDisplayName()) {
            backendSettings.setDisplayName(displayName.text === "" ? "UserName" : displayName.text);
            backendSettings.setDisplayName(displayName.text);
            sessionHandler.updateDisplayName();
        }
        else {
            backendSettings.setDisplayName(displayName.text === "" ? "UserName" : displayName.text);
            backendSettings.setDisplayName(displayName.text);
        }

        backendSettings.saveSettings();
        settings.close();
    }
    onRejected: console.log("Close clicked")
    onReset: {
        console.log("Restore Defaults clicked")
        var displayName = userHandler.getDisplayName();
        backendSettings.loadAndSaveDefaultSettings(displayName);
        loadSettings();
    }
}

