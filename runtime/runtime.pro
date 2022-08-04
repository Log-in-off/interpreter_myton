TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        runtime.cpp \
        runtime_test.cpp

HEADERS += \
    runtime.h \
    test_runner_p.h
