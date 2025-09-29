# Copyright 2020 Purdea Andrei
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = util
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    monitorwindow.cpp \
    communication.cpp \
    device.cpp \
    hidthread.cpp \
    kbd_defs.cpp \
    signal_level.cpp \
    columntester.cpp \
    rowdactester.cpp

HEADERS += \
    mainwindow.h \
    monitorwindow.h \
    communication.h \
    device.h \
    hidthread.h \
    kbd_defs.h \
    signal_level.h \
    columntester.h \
    rowdactester.h

unix:!macx {
    #LIBS += -lhidapi-libusb
    LIBS += -lhidapi-hidraw
}

macx {
    # tip: build and install hidapi manually with desired MACOSX_DEPLOYMENT_TARGET
    HIDAPI_PREFIX = $$(HIDAPI_PREFIX)
    isEmpty( HIDAPI_PREFIX ) {
        HIDAPI_PREFIX = /opt/homebrew
    }
    INCLUDEPATH += $(HIDAPI_PREFIX)/include
    LIBS +=      -L$(HIDAPI_PREFIX)/lib   -lhidapi

    # assume SDK 26 or later, effectively remove "-framework AGL" which qt5 cmake adds
    QMAKE_LIBS_OPENGL = -framework OpenGL
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.0

    ICON = modelfkey.icns
}

win32 {
    LIBS += -lhidapi -lsetupapi
    RC_ICONS = modelfkey.ico
}

FORMS += \
    mainwindow.ui \
    monitorwindow.ui \
    signal_level.ui \
    columntester.ui \
    rowdactester.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
