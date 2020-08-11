import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C

Dialog {
    id: dialogId
    width: 600
    height: 600
    //modal: true
    title: "Server Information"
    //anchors.centerIn: parent
    standardButtons: Dialog.Close | Dialog.Apply

    Rectangle{
        //anchors.fill: parent

        ColumnLayout {
            //anchors.fill: parent
            spacing: 10
            Text {
                text: qsTr("We couldn't find a server to connect to! \n Please fill in the details below.")
                anchors.horizontalCenter: dialogId.horizontalCenter
            }

            RowLayout {
                Text {
                    id: hostAddressText
                    text: qsTr("Host Address:");
                }
                TextField {
                    id: hostAddressTextField
                    placeholderText: "Host Address"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    text: qsTr("TCP Port:");
                }
                TextField {
                    id: tcpPortTextField
                    placeholderText: "1337"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    //id: udpPortTextField
                    text: qsTr("UDP Port:");
                }
                TextField {
                    id: udpPortTextField
                    placeholderText: "1338"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            RowLayout {
                Text {
                    //id: hostAddressText
                    text: qsTr("SQL TCP Port:");
                }
                TextField {
                    id: sqlTcpPortTextField
                    placeholderText: "1339"
                    cursorVisible: false
                    maximumLength: 20
                }
            }

            Text {
                id: error
                visible: false
                color: "red"
            }

        }
    }
    onRejected: console.log("Close clicked")
    onApply: {
        if (udpPortTextField.text.length > 0 && sqlTcpPortTextField.text.length > 0 && tcpPortTextField.text.length > 0  && hostAddressTextField.text.length > 0) {
            backendSettings.setServerIpAddress(hostAddressTextField.text);
            backendSettings.setTcpPort(tcpPortTextField.text);
            backendSettings.setUdpPort(udpPortTextField.text);
            backendSettings.setSqlTcpPort(sqlTcpPortTextField.text);
            dialogId.close()
        } else {
            error.text = "Please fill in all the fields"
            error.visible = true
        }
    }
}
