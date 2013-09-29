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

win32-* {
    CONFIG += static debug
}

SOURCES += src/main.cpp \
    src/utils.cpp \
    src/PreTokenizer.cpp \
    src/unicode.cpp \
    src/Tokenizer.cpp \
    src/SimpleExpressionParser.cpp

HEADERS += \
    src/utils.hpp \
    src/constants.hpp \
    src/PreTokenizer.hpp \
    src/Tokenizer.hpp \
    src/unicode.hpp \
    src/IPreTokenStream.hpp \
    src/DebugPreTokenStream.hpp \
    src/DebugTokenOutputStream.hpp \
    src/SimpleExpressionParser.hpp

