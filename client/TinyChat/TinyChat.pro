QT       += core gui multimedia network
LIBS     += -lWs2_32
INCLUDEPATH += $$PWD/jsoncpp
include($$PWD/jsoncpp/jsoncpp.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += PRECOMPILED_HEADER

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addfrienddlg.cpp \
    basic.cpp \
    chattextbrowser.cpp \
    chattextedit.cpp \
    groupchatbutton.cpp \
    groupchatpage.cpp \
    logindlg.cpp \
    main.cpp \
    privatechatbutton.cpp \
    privatechatpage.cpp \
    registerdlg.cpp \
    task.cpp \
    tododlg.cpp \
    userinfodlg.cpp \
    widget.cpp

HEADERS += \
    addfrienddlg.h \
    basic.h \
    chattextbrowser.h \
    chattextedit.h \
    groupchatbutton.h \
    groupchatpage.h \
    logindlg.h \
    msgDef.h \
    privatechatbutton.h \
    privatechatpage.h \
    registerdlg.h \
    task.h \
    tododlg.h \
    userinfodlg.h \
    widget.h

FORMS += \
    addfrienddlg.ui \
    logindlg.ui \
    registerdlg.ui \
    tododlg.ui \
    userinfodlg.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    TinyChat.qrc

DISTFILES += \
    source/QQ message alert tone.wav
