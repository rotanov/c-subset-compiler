TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -static

DESTDIR = bin

CONFIG(debug, debug|release) {
    DEFINES += \
        _DEBUG
    OBJECTS_DIR = temp/debug/obj
    TARGET = compiler-debug
} else {
    OBJECTS_DIR = temp/release/obj
    TARGET = compiler-release
}

SOURCES += main.cpp \
    utils.cpp \
    pretokenizer.cpp \
    unicode.cpp \
    tokenizer.cpp

HEADERS += \
    utils.hpp \
    constants.hpp \
    pretokenizer.hpp \
    tokenizer.hpp \
    unicode.hpp \
    ipretokenstream.hpp \
    debugpretokenstream.hpp \
    debugtokenoutputstream.hpp

