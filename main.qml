import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
//import QtQuick 2.0
//import VLCQt 1.1
//import QtQuick.Controls 1.4
import QtQuick.Layouts 1.15
import "components" as C

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 720
    minimumHeight: 400
    minimumWidth: 400
    title: qsTr("qZoom")
   // Rectangle{
       // width: 400
    /*
    Column{
        spacing: 2
        Image {
            id: liveImage
            property bool counter: false

            asynchronous: true
            source: "image://live/10"
            //anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            cache: false
            width: 480
            height: 360

            function reload() {
                counter = !counter
                source = "image://live/image?id=" + counter + "&0"
            }
        }
        Item {
            Timer {
                interval: 41; running: true; repeat: true
                onTriggered: liveImage.reload();
            }
        }
        Image {
            id: liveImage2
            property bool counter: false

            asynchronous: true
            source: "image://live/10"
            //anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            cache: false
            width: 480
            height: 360

            function reload() {
                counter = !counter
                source = "image://live/image?id=" + counter + "&1"
            }
        }
        Item {
            Timer {
                interval: 41; running: true; repeat: true
                onTriggered: liveImage2.reload();
            }
        }
    }


    Button {
        text: "Add Screen"
        onClicked: {
        }
    }
    */

    property var focusScreen: false;
    property var selectedScreenIndex: 0

    Rectangle {
        id: screenGridArea
        width: window.width
        height: (window.height - taskBar.height)
        color: "dimgray"
        GridLayout {
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
            Repeater {
                model: focusScreen ? 1 : imageHandler.getNumberOfScreens()
                Rectangle {
                    width: focusScreen ? screenGridArea.width : screenGridArea.width/parent.columns
                    height: focusScreen ? screenGridArea.height : screenGridArea.height/parent.rows
                    color: "dimgray"

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
                            //console.log(screenIndex)
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
            }
        }
    }

    C.TaskBar {
        id: taskBar
        anchors.bottom: parent.bottom

    }


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
