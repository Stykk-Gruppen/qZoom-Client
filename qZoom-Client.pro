QT += quick multimedia core multimediawidgets network

CONFIG += c++17 console

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -L/usr/lib -lavdevice -lavformat -lavutil -lswscale -lswresample -lavcodec -lavfilter -I/usr/include -lao
TARGET.CAPABILITY += SwEvent

SOURCES += \
        core/servertcpqueries.cpp \
        core/systemcall.cpp \
        handlers/audiohandler.cpp \
        handlers/audioplaybackhandler.cpp \
        config.cpp \
        handlers/errorhandler.cpp \
        handlers/outputstreamhandler.cpp \
        handlers/sessionhandler.cpp \
        handlers/userhandler.cpp \
        handlers/imagehandler.cpp \
        handlers/inputstreamhandler.cpp \
        main.cpp \
        participant.cpp \
        playback.cpp \
        settings.cpp \
        handlers/udpsockethandler.cpp \
        handlers/tcpsockethandler.cpp \
        handlers/videohandler.cpp \
        handlers/videoplaybackhandler.cpp

RESOURCES += qml.qrc

#INCLUDEPATH += /usr/include



# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

QT_QML_DEBUG

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    core/servertcpqueries.h \
    core/systemcall.h \
    handlers/audiohandler.h \
    handlers/audioplaybackhandler.h \
    handlers/errorhandler.h \
    handlers/outputstreamhandler.h \
    handlers/sessionhandler.h \
    handlers/userhandler.h \
    handlers/imagehandler.h \
    handlers/inputstreamhandler.h \
    participant.h \
    playback.h \
    settings.h \
    handlers/udpsockethandler.h \
    handlers/tcpsockethandler.h \
    handlers/videohandler.h \
    handlers/videoplaybackhandler.h

DISTFILES += \
    img/qZoom-logo.png
