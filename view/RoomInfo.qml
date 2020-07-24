import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C

Dialog {
    id: roomInfo
    visible: false
    width: 400
    height: 300
    //modal: true
    title: "Room Information"
    //anchors.centerIn: parent
    standardButtons: Dialog.Close

    /*
    function updateValues() {
        roomIdText.text = sessionHandler.getRoomId()
        roomPasswordText.text = sessionHandler.getRoomPassword()


    }
    */



    Rectangle{

        ColumnLayout {
            RowLayout {
                spacing: 10
                Text {
                    text: qsTr("Room ID:")
                }
                Text {
                    id: roomIdText
                    text: sessionHandler.getRoomId()
                }
            }

            RowLayout {
                spacing: 10
                Text {
                    id: roomPasswordText
                    text: qsTr("Room Password:");
                }
                Text {
                    text: sessionHandler.getRoomPassword()
                }
            }

            RowLayout {
                spacing: 10
                Text {
                    id: roomHostText
                    text: qsTr("Room Host:");
                }
                Text {
                    text: sessionHandler.getRoomHostUsername()
                }
            }
        }
    }
    onRejected: console.log("Close clicked")
}
