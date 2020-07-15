import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    color: "#141414"
    height: 64
    width: parent.width

    /*

    Row {

        Rectangle {
            width: 32
            height: 32
            color: "white"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    cb.visible = !cb.visible
                }
            }





            ComboBox {
                id: cb
                width: 200
                visible: false
                model: ListModel {
                    id: t
                    function getAudioTypes() {
                        var inputList = audioHandler.getAudioInputDevices()
                        clear()
                        for (var title in inputList) {
                            append({text: inputList[title]})
                        }
                    }
                }
                onActivated: {
                    //console.log("switched Audio Input Device to: " + currentValue)
                    audioHandler.changeAudioInputDevice(currentValue);
                }
            }
        }


    }

    Component.onCompleted: {
        t.getAudioTypes()

    }
    */

}
