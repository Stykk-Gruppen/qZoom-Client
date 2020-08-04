import QtQuick 2.0


Rectangle {
    //color: parent.color
    width: parent.width
    height: Math.min(parent.height / 5, 150)


    Column {
        height: parent.height
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
            anchors.verticalCenter: parent.bottom
        }
    }

    Row {
        //anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        //anchors.rightMargin: 1000
        spacing: 32

        Rectangle {
            id: settingsRectangle
            color: "#5CBB5C"
            implicitHeight: 50
            implicitWidth: 50
            radius: 11
            //anchors.horizontalCenter: parent.horizontalCenter
            //anchors.verticalCenter: parent.verticalCenter
            //anchors.right: parent.right
            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                height: 25
                width: 25
                source: "../img/settings-icon.png"
            }

            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: showSettings()
            }
        }

        Rectangle {
            id: logOutRectangle
            visible: false
            color: "#5CBB5C"
            implicitHeight: 50
            implicitWidth: 50
            radius: 11

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                height: 25
                width: 25
                source: "../img/logout-icon.png"
            }

            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
            }
        }
    }


}
