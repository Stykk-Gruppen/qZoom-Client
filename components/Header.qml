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
    }

    Row {

        width: parent.width - 30
        height: parent.heigt
        //anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        layoutDirection: Qt.RightToLeft
        anchors.rightMargin: 15
        anchors.horizontalCenterOffset: -20
        spacing: 15

        Rectangle {
            id: logoutRectangle
            visible: !userHandler.isGuest()
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
                onClicked: {
                    if (userHandler.logout()) {
                        console.log("Successfully logged out");
                        changePage("home");
                    }
                }
            }
        }

        Rectangle {
            id: settingsRectangle
            color: "#5CBB5C"
            implicitHeight: 50
            implicitWidth: 50
            radius: 11
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


    }

    Rectangle {
        id: line
        height: 5
        width: parent.width
        color: "#5CBB5C"
        anchors.bottom: parent.bottom
    }

    function showLogoutButton() {
        logoutRectangle.visible = true;
    }

}
