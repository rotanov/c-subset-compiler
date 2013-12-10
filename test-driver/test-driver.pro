QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

DESTDIR = ../bin

INCLUDE = $$(INCLUDE)
INCLUDE = $$split(INCLUDE, ;)
for (path, INCLUDE):QMAKE_CXXFLAGS += -isystem $${path}

LIB = $$(LIB)
LIB = $$split(LIB, ;)
for(path, LIB):LIBS += -L$${path}

#!build_pass:message($$LIBS)

DEFINES += BOOST_ALL_NO_LIB

# TODO: investigte why it doesn't work
#QMAKE_CXXFLAGS += -fsanitize=undefined
QMAKE_CXXFLAGS += -Werror=return-type
QMAKE_CXXFLAGS += -Werror=non-virtual-dtor
QMAKE_CXXFLAGS += -Werror=parentheses

QMAKE_CXXFLAGS += -Wextra

# GCC handles C++11 class members inline
# initialization wrong in context of warnings
QMAKE_CXXFLAGS += -Wno-reorder

#QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

INCLUDEPATH += ../src

CONFIG(debug, debug|release) {
    DEFINES += \
        _DEBUG
    TARGET = test-driver-debug
} else {
    TARGET = test-driver-release

#    To add debug information to release build:
#    QMAKE_CXXFLAGS_RELEASE += -g
#    QMAKE_CXXFLAGS_RELEASE -= -O2
#    QMAKE_LFLAGS_RELEASE -= -Wl,-s
}

LIBS += \
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
    ../src/ExpressionParser.cpp \
    ../src/Token.cpp \
    ../src/Parser.cpp \
    ../src/ASTNode.cpp \
    ../src/SymbolTable.cpp \
    ../src/prettyPrinting.cpp \
    ../src/Statement.cpp

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
    ../src/ExpressionParser.hpp \
    ../src/Token.hpp \
    ../src/Parser.hpp \
    ../src/ASTNode.hpp \
    ../src/SymbolTable.hpp \
    ../src/prettyPrinting.hpp \
    ../src/Statement.hpp

FORMS    += mainwindow.ui
