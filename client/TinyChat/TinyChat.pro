QT       += core gui network
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
    basic.cpp \
    chattextbrowser.cpp \
    chattextedit.cpp \
    logindlg.cpp \
    main.cpp \
    privatechatbutton.cpp \
    privatechatpage.cpp \
    registerdlg.cpp \
    task.cpp \
    userinfodlg.cpp \
    widget.cpp

HEADERS += \
    basic.h \
    chattextbrowser.h \
    chattextedit.h \
    logindlg.h \
    msgDef.h \
    privatechatbutton.h \
    privatechatpage.h \
    registerdlg.h \
    task.h \
    userinfodlg.h \
    widget.h

FORMS += \
    logindlg.ui \
    registerdlg.ui \
    userinfodlg.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    TinyChat.qrc
