QT       += core gui  serialbus serialport   openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11

# Include paths for OpenCV
INCLUDEPATH += "/usr/include/opencv4"

INCLUDEPATH +="/usr/include"
INCLUDEPATH +="/usr/include/glib-2.0"
INCLUDEPATH +="/usr/include/gstreamer-1.0"
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0
PKGCONFIG += gstreamer-video-1.0

INCLUDEPATH +="/usr/local/cuda-11.2/targets/x86_64-linux/include"

INCLUDEPATH +="/opt/nvidia/deepstream/deepstream-6.4/sources/includes"


# Library paths and OpenCV libraries
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
LIBS += -L/usr/local/cuda-11.2/lib64 -lcudart
LIBS += -L/opt/nvidia/deepstream/deepstream-6.4/lib -lnvdsgst_meta -lnvds_meta
#LIBS += -L/usr/lib/aarch64-linux-gnu/tegra -lnvbufsurface -lnvbufsurftransform


LIBS += -lmodbus  -lboson -lFSLP
LIBS+=-L"/usr/lib/x86_64-linux-gnu/gstreamer-1.0" -lgstxvimagesink -L"/usr/lib/x86_64-linux-gnu" -lgstbase-1.0 -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0
# Library paths and libraries
LIBS += -L/usr/local/lib #-lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc

# GStreamer and plugins libraries
LIBS += -lgstreamer-1.0 -lgstapp-1.0 -lgstbase-1.0 -lgobject-2.0 -lglib-2.0
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camerathread.cpp \
    camerawidget.cpp \
    dataprocessor.cpp \
    daycameracontrolthread.cpp \
    gpssniffthread.cpp \
    lensnightcameracontrolthread.cpp \
    lrfcontrolthread.cpp \
    main.cpp \
    mainwindow.cpp \
    nightcameracontrolthread.cpp \
    plccontrolthread.cpp \
    radarsniffthread.cpp \
    serialportmanager.cpp \
    servoactuatorcontrolthread.cpp \
    servodrivercontrolthread.cpp \
    viscacontrol.cpp

HEADERS += \
    camerathread.h \
    camerawidget.h \
    datamodel.h \
    dataprocessor.h \
    daycameracontrolthread.h \
    gpssniffthread.h \
    lensnightcameracontrolthread.h \
    lrfcontrolthread.h \
    mainwindow.h \
    nightcameracontrolthread.h \
    objectinfo.h \
    plccontrolthread.h \
    radarsniffthread.h \
    serialportmanager.h \
    servoactuatorcontrolthread.h \
    servodrivercontrolthread.h \
    viscacontrol.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    dstest1_pgie_config.txt \
    pgie_config.txt
