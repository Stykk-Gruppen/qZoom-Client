import QtQuick 2.0

Rectangle {
    width: focusScreen ? screenGridArea.width : screenGridArea.width/parent.columns
    height: focusScreen ? screenGridArea.height : screenGridArea.height/parent.rows
    color: "#161637"
    property var index: 0

    function setInfo(_index, _width, _height) {
        index = _index;
        width = _width;
        height = _height;
    }


    //Brukes dette?
    //Nei. Hvem spurte?
    Image {
        id: liveImage
        anchors.fill: parent
        property bool counter: false

        asynchronous: true
        source: "image://live/10"
        fillMode: Image.PreserveAspectFit
        cache: false

        function reload() {
            counter = !counter
            var screenIndex = focusScreen ? selectedScreenIndex : index
            console.log(screenIndex)
            source = "image://live/image?id=" + counter + "&" + screenIndex
        }

        MouseArea {
            anchors.fill: parent
            onDoubleClicked: {
                selectedScreenIndex = index
                focusScreen = !focusScreen
            }
        }
    }

    Item {
        Timer {
            interval: 41; running: true; repeat: true
            onTriggered: liveImage.reload();
        }
    }
}
