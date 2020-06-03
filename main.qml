import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5

ApplicationWindow {
    id: window
    visible: true
    width: 1400
    height: 800
    title: qsTr("qZoom")

    Item {
       anchors.fill: parent

        Camera {
            id: camera
            deviceId: cameraHandler.getDeviceId()

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
        }
    }
}
