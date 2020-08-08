import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C

Dialog {
    id: roomInfo
    visible: false
    width: 400
    height: 300
    //modal: true
    title: "Kick user"
    //anchors.centerIn: parent
    standardButtons: Dialog.Close | Dialog.Apply

    /*
    function updateValues() {
        roomIdText.text = sessionHandler.getRoomId()
        roomPasswordText.text = sessionHandler.getRoomPassword()


    }
    */

    Connections {
        target: imageHandler
        function onRefreshScreens() {
            participantModel.applyFilter();
        }
    }

    ListModel {
        id: participantModel
        function applyFilter() {
            var participantList = imageHandler.getAllParticipantsDisplayNames();
            participantModel.clear();
            for (var title in participantList) {
                var valueList = participantList[title];
                append({name : valueList});
                console.log(valueList)
            }
        }
    }

    Rectangle{
        ColumnLayout {
            spacing: 16

            Text {
                text: "The other participants in the session:"
            }

            TableView {
                id: tableView
                width: 200
                height: 200
                TableViewColumn {
                    role: "name"
                    title: "Participant"
                    width: 200
                }
                model: participantModel
            }
        }
    }
    onRejected: console.log("Close clicked")
    onApply: {
        sessionHandler.kickParticipant(tableView.currentRow)
    }

    Component.onCompleted: {
        participantModel.applyFilter();
    }
}
