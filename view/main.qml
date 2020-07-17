import QtQuick.Window 2.12
import QtQuick 2.12
import QtQuick.Controls 2.3
import "../components" as C

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1280
    height: 720
    minimumHeight: 400
    minimumWidth: 400
    title: qsTr("qZoom")

    StackView{
        id: stackView
        focus: true
        anchors.fill: parent

        replaceEnter: Transition {

                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to:1
                    duration: 0
                }
            }
            replaceExit: Transition {
                PropertyAnimation {
                    property: "opacity"
                    duration: 0
                    from: 1
                    to:0

                }
            }
    }

    // After loading show initial Login Page
    Component.onCompleted: {
        stackView.push("qrc:/view/home.qml")
    }

    function pushPage(page) {
        var template = "qrc:/view/%1.qml";
        var url = template.arg(page);

        stackView.push(url);
    }

    function popToPage(page) {
        var template = "qrc:/view/%1.qml";
        var url = template.arg(page);

        stackView.pop(url);
    }

    function changePage(page) {
        var template = "qrc:/view/%1.qml";
        var url = template.arg(page);

        stackView.replace(url);
    }

    function setTitle(title) {
        rootWindow.title = title;
    }
}
