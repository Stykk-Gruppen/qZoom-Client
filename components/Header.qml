import QtQuick 2.0

Rectangle {
    //color: parent.color
    width: parent.width
    height: 150

    Column {
        spacing: 50
        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 50
        Image {
            id: logo
            source: "../img/qZoom-logo.png"
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            sourceSize.height: 50
        }

        Rectangle {
            id: line
            height: 5
            width: parent.width
            color: "#5CBB5C"
        }
    }
}
