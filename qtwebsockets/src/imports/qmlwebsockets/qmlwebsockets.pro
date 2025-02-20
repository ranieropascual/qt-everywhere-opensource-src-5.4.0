QT = core websockets qml

TARGETPATH = Qt/WebSockets

HEADERS +=  qmlwebsockets_plugin.h \
            qqmlwebsocket.h \
            qqmlwebsocketserver.h

SOURCES +=  qmlwebsockets_plugin.cpp \
            qqmlwebsocket.cpp \
            qqmlwebsocketserver.cpp

OTHER_FILES += qmldir

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

load(qml_plugin)
