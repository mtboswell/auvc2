#include "modulehub.h"


ModuleHub::ModuleHub(AUVC_State_Data* stateIn, QStringList* debugIn, bool master){
	state = stateIn;
	config = configIn;
	debug = debugIn;
	// connect up server to state data
	if(master){
		srv = new VDataServer();
	}
	else{
		srv = new VDataClient();
	}
	connect(srv, SIGNAL(datumReceived(VDatum, QHostAddress)), this, SLOT(messageIn(VDatum)));
	connect(this, SIGNAL(messageBroadcast(VDatum)), srv, SLOT(sendVDatum(VDatum)));

	connect(srv, SIGNAL(connectionRestored(QTime)), state, SLOT(sync(QTime)));
	connect(srv, SIGNAL(syncData()), state, SLOT(sync()));
	connect(state, SIGNAL(syncData(VDatum)), srv, SLOT(sendVDatum(VDatum)));

	connect(state, SIGNAL(dataEdited(VDatum)), this, SLOT(editIn(VDatum)));
}
/**
 * addModule adds a module object to the vdata broadcasting system.
 * \param module pointer to module QObject
 */
void ModuleHub::addModule(Module* module){
	const QMetaObject* metaMod = module->metaObject();
	qDebug() << "Initializing Module:" << metaMod->className();

	bool thisDebug = (*debug).contains(metaMod->className());
	module->setLinks(state, thisDebug);
	if(thisDebug) qDebug() << "Activiating debugging for " << metaMod->className();

	connect(this, SIGNAL(subOut(QString, VDatum)), module, SLOT(recvDatum(QString, VDatum)));
	addSubscriptions(metaMod->className(), module->subscriptions());

	connect(module, SIGNAL(sendData(VDatum)), this, SLOT(moduleIn(VDatum)));
	if(module->isThread()) connect(this, SIGNAL(go()), module, SLOT(start()));

}
/**
 * As an alternative to the addModule function, you can set this module as the parent of your modules, then call this function.
 */
void ModuleHub::initializeAndLaunchAllModules(){
	QObjectList kids = this->children();
	foreach(QObject* kid, kids){
		if(kid->inherits("Module")) addModule((Module*)kid);
		qDebug() << "Adding Module";
	}
	qDebug() << "Module Initialization Complete, Starting Threads";
	startAll();
}

void ModuleHub::addSubscriptions(QString module, QStringList IDs){
	foreach(QString str, IDs){
		subscriptions[str].append(module);
	}
}
void ModuleHub::addSubscription(QString module, QString ID){
	subscriptions[ID].append(module);
}

void ModuleHub::notify(VDatum msg){
	QStringList modulesToNotify;


	QMapIterator<QString, QStringList> i(subscriptions); 
	while(i.hasNext()){
		i.next();
		if(msg.id.startsWith(i.key())) modulesToNotify.append(i.value());
	}

	modulesToNotify.removeDuplicates();

	// handle subscriptions
	foreach(QString str, modulesToNotify){
		emit subOut(str, msg);
	}
}

void ModuleHub::moduleIn(VDatum msg){
	//qDebug() << "Message ID:" << msg.id;
	state->setData(msg);
	notify(msg);
	messageBroadcast(msg);
}

void ModuleHub::messageIn(VDatum msg){
	//qDebug() << "Message ID:" << msg.id;
	state->setData(msg);
	notify(msg);
}

void ModuleHub::editIn(VDatum msg){
	notify(msg);
	messageBroadcast(msg);
}

void ModuleHub::sync(){
	//srv->sync(QTime::currentTime().addSecs(-300));
	srv->sync();
}
