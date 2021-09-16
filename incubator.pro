QT       += core gui sql network charts printsupport serialport multimedia serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

RC_ICONS = icon.ico
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    box.cpp \
    devicecom.cpp \
    devicespull.cpp \
    dialog.cpp \
    dialogpassword.cpp \
    history.cpp \
    main.cpp \
    mainwindow.cpp \
    mysortfilterproxymodel.cpp \
    netread.cpp \
    readbdthread.cpp \
    report.cpp \
    savesettings.cpp \
    settings.cpp

HEADERS += \
    TypeData.h \
    box.h \
    devicecom.h \
    devicespull.h \
    dialog.h \
    dialogpassword.h \
    history.h \
    mainwindow.h \
    mysortfilterproxymodel.h \
    netread.h \
    readbdthread.h \
    report.h \
    savesettings.h \
    settings.h

FORMS += \
    dialog.ui \
    dialogpassword.ui \
    history.ui \
    mainwindow.ui \
    report.ui \
    settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=
