import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import "../components" as C

Rectangle {
    id: window
    visible: true

    property var focusScreen: false;
    property var selectedScreenIndex: 0

    C.RoomInfo {
        id: roomInfo
    }

    C.KickUser {
        id: kickUser
    }

    Rectangle {
        id: screenGridArea
        width: window.width
        height: (window.height - taskBar.height)
        color: "#161637"
        Grid {
            id: gridId
            function calcColumns() { //Disse to funksjonene kan nok gjøres mye bedre. Hvis man klarer å se mønsteret :P
                var a = imageHandler.getNumberOfScreens()
                if (a > 9){
                    return 4;
                }
                else if (a > 4){
                    return 3;
                }
                else if (a > 1){
                    return 2;
                }
                else {
                    return 1;
                }
            }
            function calcRows() {
                var a = imageHandler.getNumberOfScreens()
                if (a > 6){
                    return 3;
                }
                else if (a > 2){
                    return 2;
                }
                else {
                    return 1;
                }
            }
            columns: calcColumns()
            rows: calcRows()

            columnSpacing: 0
            rowSpacing: 0


            Repeater {
                id: repeaterId
                model: focusScreen ? 1 : imageHandler.getNumberOfScreens()
                Rectangle {
                    width: focusScreen ? screenGridArea.width : screenGridArea.width/parent.columns
                    height: focusScreen ? screenGridArea.height : screenGridArea.height/parent.rows
                    color: "#161637"

                        Image {
                            id: liveImage
                            width: parent.width - 4
                            height: parent.height - 4
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            property bool counter: false
                            layer.enabled: true
                            asynchronous: true
                            source: "image://live/10"
                            fillMode: Image.PreserveAspectFit
                            cache: false

                            Rectangle {
                                id: borderRectangle
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: parent.paintedWidth + 4
                                height: parent.paintedHeight + 4

                                anchors.margins: -border.width
                                z: -1
                                border.width: 4
                                border.color: "red"
                                color: "#161637"
                            }

                            function reload() {
                                counter = !counter
                                var screenIndex = focusScreen ? selectedScreenIndex : index
                                //console.log(screenIndex)
                                source = "image://live/image?id=" + counter + "&" + screenIndex
                                if (imageHandler.getAudioIsDisabled(index)) {
                                    borderRectangle.border.color = "#DB504A";
                                }
                                else {
                                    if (imageHandler.getIsTalking(index)) {
                                        borderRectangle.border.color = "#FFB800"
                                    }
                                    else {
                                        borderRectangle.border.color = Qt.rgba(27/255, 29/255, 54/255, 1)
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onDoubleClicked: {
                                    selectedScreenIndex = index
                                    focusScreen = !focusScreen
                                    repeaterId.model = focusScreen ? 1 : imageHandler.getNumberOfScreens()
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
            }
        }
    }

    Connections {
        target: imageHandler
        function onForceLeaveSession() {
            console.log("inside qml");
            sessionHandler.leaveSession();
            changePage("home");
        }
    }

    C.TaskBar {
        id: taskBar
        anchors.bottom: parent.bottom
    }

    Component.onCompleted: {
        //addScreen();
        var roomId = sessionHandler.getRoomId();
        var roomPassword = sessionHandler.getRoomPassword();
        setTitle("qZoom :: Session (" + roomId + ")");

        //backendSettings.setLastRoomId(roomId);
        //backendSettings.setLastRoomPassword(roomPassword);
        //backendSettings.saveSettings();

        /*if(backendSettings.getAudioOn())
        {
            streamHandler.enableAudio();
        }
        if(backendSettings.getVideoOn())
        {
            streamHandler.enableVideo();
        }*/
    }
    /*
    function addScreen() {
        var width = screenGridArea.width/gridId.calcColumns();
        var height = screenGridArea.height/gridId.calcRows()
        console.log("width" + width);
        console.log("height" + height);
        for (var i = 0; i < imageHandler.getNumberOfScreens(); i++) {
            var component = Qt.createComponent("../components/Screen.qml");
            if (component.status === Component.Ready) {
                var button = component.createObject(gridId);
                button.setInfo(i, width, height);
            }
        }
        console.log("Number of Screens: " + imageHandler.getNumberOfScreens());
    }
    */
}


/*VideoOutput {
        id: test
        width: 1920
        height: 1080
        source: mediaplayer
    }



    ComboBox {
        width: 200
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

    Component.onCompleted: {
        t.getAudioTypes()

    }


/*
        Camera {
            id: camera
            objectName: "qrCameraQML"
            imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash
            exposure {
                exposureCompensation: -1.0
                exposureMode: Camera.ExposurePortrait
            }

            flash.mode: Camera.FlashRedEyeReduction
            captureMode: Camera.CaptureVideo
            imageCapture {
                onImageCaptured: {
                    photoPreview.source = preview  // Show the preview in an Image

                }
            }
        }

        VideoOutput {
            source: camera
            anchors.fill: parent
            focus : visible // to receive focus and capture key events when visible
        }

        Image {
            id: photoPreview
        } */

/*ComboBox {
                width: 200
                model: ListModel {
                    id: t
                    function getAudioTypes() {
                        var inputList = cameraTest.getAudioInputDevices()
                        clear()
                        for (var title in inputList) {
                            append({text: inputList[title]})
                        }
                    }
                }
                onActivated: {
                    //console.log("switched Audio Input Device to: " + currentValue)
                    //cameraTest.changeAudioInputDevice(currentValue);
                }
            }

            Component.onCompleted: t.getAudioTypes()*/


/*    }

*/
