import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5
//import QtQuick 2.0
//import VLCQt 1.1

ApplicationWindow {
    id: window
    visible: true
    width: 1400
    height: 800
    title: qsTr("qZoom")

    Image {
      id: liveImage
      property bool counter: false

      asynchronous: true
      source: "image://live/10"
      anchors.fill: parent
      fillMode: Image.PreserveAspectFit
      cache: false

      function reload() {
        counter = !counter
        source = "image://live/image?id=" + counter
      }
    }

    Item {
        Timer {
            interval: 20; running: true; repeat: true
            onTriggered: liveImage.reload();
        }
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
