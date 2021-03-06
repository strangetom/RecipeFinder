#-------------------------------------------------
#
# Project created by QtCreator 2016-09-24T17:25:16
#
#-------------------------------------------------

QT       += core gui sql
QMAKE_LFLAGS += -no-pie # so ubuntu doesn't see it as a shared library

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webenginewidgets

TARGET = RecipeFinder
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    database.cpp

HEADERS  += mainwindow.h \
    fts_fuzzy_match.h \
    database.h

FORMS    +=
