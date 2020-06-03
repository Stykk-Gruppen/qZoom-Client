import QtQuick 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.15
import QtQuick.Controls 2.5


ApplicationWindow {
    id: window
    visible: true
    width: 1400
    height: 800
    title: qsTr("Stack")


    Item {
        width: 1400
        height: 800

        Camera {
            id: camera

            imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash
            focus {
                        focusMode: Camera.FocusMacro
                        focusPointMode: Camera.FocusPointCustom
                        customFocusPoint: Qt.point(0.2, 0.2) // Focus relative to top-left corner
                    }
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
