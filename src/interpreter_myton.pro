TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        lexer.cpp \
        lexer_test_open.cpp \
        main.cpp

HEADERS += \
    lexer.h \
    test_runner_p.h
