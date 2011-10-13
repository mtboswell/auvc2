# comment out this line to enable warnings
CONFIG += warn_off

# determine distro
ISSUE = $$system(cat /etc/issue)
contains( ISSUE, [uU]buntu ) {
	CONFIG += ubuntu
}
contains( ISSUE, [aA]rch ){
	CONFIG += arch
}
contains( ISSUE, [fF]edora ){
	CONFIG += fedora
}

TEMPLATE = app
TARGET = ../../bin/agent
CONFIG += debug
QT += network 
MOC_DIR = build/
OBJECTS_DIR = build/
UI_DIR = build/

# llua-5.1
ubuntu {
	LIBS +=  -llua5.1
} else {
	LIBS +=  -llua
}

# old libs
#LIBS += -Lserver-jaus -ljauscore -ljausmobility -ltinyxml -lcxutils

DEPENDPATH += . \

INCLUDEPATH += . \
	     ../framework \
	     ../modules 

# Main ####################################################
HEADERS += \
           ../framework/module/module.h \
           ../framework/module/modulehub.h \
           ../framework/state/state.h \
           ../framework/state/vdatum.h \
           ../framework/state/vdatasocket.h \
           ../framework/state/treemodel/treeitem.h \
           ../framework/state/treemodel/treemodel.h 

SOURCES += \
           main.cpp \
           ../framework/module/module.cpp \
           ../framework/module/modulehub.cpp \
           ../framework/state/vdatum.cpp \
           ../framework/state/vdatasocket.cpp \
           ../framework/state/treemodel/treeitem.cpp \
           ../framework/state/treemodel/treemodel.cpp 

