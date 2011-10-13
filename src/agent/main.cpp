#define GUI

#ifdef GUI
#include <QApplication>
#include <QTreeView>
#else
#include <QCoreApplication>
#endif

#include <QDebug>
#include <QMutex>

#include "misc/configloader.h"

#include "state/state.h"
#include "state/vdatum.h"
#include "module/modulehub.h"

// modules
#define MODULE(mod) #include "../modules/mod"
#include "modules"
#undef MODULE

int main(int argc, char *argv[]){

#ifdef GUI
	QApplication app(argc, argv);
#else
	QCoreApplication app(argc, argv);
#endif
	// required for module settings
	QCoreApplication::setOrganizationName("AUVT");
	QCoreApplication::setOrganizationDomain("auvt.org");
	QCoreApplication::setApplicationName("AUVC2");

	// arg controlled vars
	bool simulate = false;

	// set arg controlled vars
	QStringList args = app.arguments();
	QString arg;
	foreach(arg, args){
		// -s means simulate, or, more specifically, ignore hardware
		if(arg == "-s" || arg == "--simulate") simulate = true;
		else if(arg == "-h"){
			qDebug() << "Available Arguments:";
			qDebug() << "\t-s or --simulate : disable hardware";
			return 0;
		}
	}

	// Required for using VDatums in signals and slots
	qRegisterMetaType<VDatum>("VDatum");
	qRegisterMetaType<QList<VDatum> >("QList<VDatum>");

	AUVC_State_Data stateData;
	//Set inital Data
	stateData.setData("Simulate", simulate);

	ModuleHub hub(&stateData);

#define MODULE(mod) qDebug()<< "Creating mod"; hub.addModule(new mod());
#include "modules"
#undef MODULE

	/* Start everything */
	hub.initializeAndLaunchAllModules();


#ifdef GUI
	QTreeView *tree = new QTreeView();
	tree->setModel(&stateData);

	tree->show();
#endif

	// Start main event loop (for dataHub)
	return app.exec();

	/* Done, keep running forever */
	//! \todo add exit and shutdown features
}
