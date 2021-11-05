QT += widgets core gui network
QT += concurrent

# Enable c++17
win32-msvc* {
    MSVC_VER = $$(VisualStudioVersion)
    greaterThan(MSVC_VER, 15.3){
        CONFIG += C++17
        QMAKE_CXXFLAGS += /std:c++17
    }
}

TEMPLATE = app
TARGET = apkinstaller

INCLUDEPATH += $$PWD/inc \
                $$PWD/gui

win32 {
    #QMAKE_LFLAGS_WINDOWS += "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
    contains(QMAKE_TARGET.arch, x86_64) {
        CONFIG(debug, debug|release) {
            DESTDIR = $$PWD/bin/debug/x64
            TARGET = $$join(TARGET,,,d)
        } else {
            DESTDIR = $$PWD/bin/release/x64
        }
    } else {
        CONFIG(debug, debug|release) {
            DESTDIR = $$PWD/bin/debug/x86
            TARGET = $$join(TARGET,,,d)
        } else {
            DESTDIR = $$PWD/bin/release/x86
        }
    }
}

win32 {
    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L$$PWD/lib/x64
    } else {
        LIBS += -L$$PWD/lib/x86
    }
}

*msvc* { # visual studio spec filter
      QMAKE_CXXFLAGS += -MP
}

HEADERS += \
    adbwrapper.h \
    aesencryption.h \
    apkinstallworker.h \
    credentionals.h \
    gui/SequryLineEdit.h \
    gui/customerinfowgt.h \
    gui/deviceinfowgt.h \
    gui/errorhandlingdlg.h \
    gui/finishedwgt.h \
    gui/installingwgt.h \
    settingkeys.h \
    installfilesgenerator.h \
    rsaencryption.h \
    gui/loginsignwgt.h \
    gui/loginwgt.h \
    gui/mainwidget.h \
    gui/passwordwgt.h \
    gui/stepwgt.h \
    gui/utils.h \
    gui/signwgt.h


SOURCES += \
    adbwrapper.cpp \
    aesencryption.cpp \
    apkinstallworker.cpp \
    credentionals.cpp \
    gui/customerinfowgt.cpp \
    gui/deviceinfowgt.cpp \
    gui/errorhandlingdlg.cpp \
    gui/finishedwgt.cpp \
    gui/installingwgt.cpp \
    installfilesgenerator.cpp \
    main.cpp \
    rsaencryption.cpp \
    gui/loginsignwgt.cpp \
    gui/loginwgt.cpp \
    gui/mainwidget.cpp \
    gui/passwordwgt.cpp \
    gui/stepwgt.cpp \
    gui/utils.cpp \
    gui/signwgt.cpp

FORMS += \
    gui/customerinfowgt.ui \
    gui/deviceinfowgt.ui \
    gui/errorhandlingdlg.ui \
    gui/finishedwgt.ui \
    gui/installingwgt.ui \
    gui/mainwidget.ui \
    gui/loginsignwgt.ui \
    gui/loginwgt.ui \
    gui/passwordwgt.ui \
    gui/signwgt.ui \
    gui/stepwgt.ui

RESOURCES += \
    res.qrc

#DEFINES += WEBAPI

DISTFILES += \
    gui/css/dark.css \
    gui/icons/Step_completed.png \
    gui/icons/Step_current.png \
    gui/icons/Step_not_completed.png \
    gui/icons/error.png \
    gui/icons/question.png \
    gui/icons/visibleOff.png \
    gui/icons/visibleOn.png


unix:!macx: LIBS += -L$$PWD/../../../../../../lib/ -lcrypto

