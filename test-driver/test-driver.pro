QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

DESTDIR = ../bin

#LIBS += $$(LIB)

INCLUDE = $$(INCLUDE)
message($$INCLUDE)
INCLUDEPATH += $$replace(INCLUDE, ;, " ")
message($$INCLUDEPATH)
#message($$replace(INCLUDEPATH, ;, " "))

#INCLUDEPATH += C:/dev/boost_1_54_0

#message(INCLUDEPATH)

DEFINES += BOOST_ALL_NO_LIB

INCLUDEPATH += ../src

CONFIG(debug, debug|release) {
    DEFINES += \
        _DEBUG
    TARGET = test-driver-debug
} else {
    TARGET = test-driver-release
}

LIBS += \
    -LC:/dev/boost_1_54_0/stage/lib \
    -lboost_system-mgw48-mt-s-1_54 \
    -lboost_regex-mgw48-mt-s-1_54 \
    -lboost_coroutine-mgw48-mt-s-1_54 \
    -lboost_context-mgw48-mt-1_54 \



SOURCES += main.cpp\
        MainWindow.cpp \
    ../src/utils.cpp \
    ../src/unicode.cpp \
    ../src/Tokenizer.cpp \
    ../src/SimpleExpressionParser.cpp \
    ../src/PreTokenizer.cpp \
    CodeEditor.cpp \
    DebugStream.cpp \
    CxxHighlighter.cpp \
    ../src/ExpressionParser.cpp

HEADERS  += MainWindow.hpp \
    ../src/utils.hpp \
    ../src/unicode.hpp \
    ../src/Tokenizer.hpp \
    ../src/SimpleExpressionParser.hpp \
    ../src/PreTokenizer.hpp \
    ../src/ITokenStream.hpp \
    ../src/IPreTokenStream.hpp \
    ../src/DebugTokenOutputStream.hpp \
    ../src/DebugPreTokenStream.hpp \
    ../src/constants.hpp \
    CodeEditor.hpp \
    DebugStream.hpp \
    CxxHighlighter.hpp \
    ../src/ExpressionParser.hpp

FORMS    += mainwindow.ui
