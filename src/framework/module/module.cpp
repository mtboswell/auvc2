/**
 * Default implementations for Module class.
 */

 #include "module.h"

 //Module::Module(AUVC_State_Data* stateIn):QThread(){ state = stateIn; }
Module::Module():QThread(){
	meta = this.metaObject();
	moduleName = meta->className();
}
	
	
void Module::setLinks(AUVC_State_Data* stateIn, bool debugIn){
	state = stateIn;
	debug = debugIn;
}

 void Module::recvDatum(QString module, VDatum datum){
 	if(module == moduleName){
		dataIn(datum);
	}
 }

void Module::setData(QString ID, QVariant value, QTime timestamp, QVariant meta){
	VDatum datum;
	datum.id = ID;
	datum.value = value;
	datum.timestamp = timestamp;
	datum.meta = meta;
	emit sendData(datum);
}
void Module::setData(VDatum datum){
	emit sendData(datum);
}
void Module::setData(QList<VDatum> datums){
	foreach(VDatum datum, datums){
		emit sendData(datum);
	}
}

void Module::setConfig(QString ID, QVariant configData){
	if(!settings.contains(moduleName+"/"+ID)){
		settings.setValue(moduleName+"/"+ID, configData);
	}
}

void Module::run(){
	stepTimer = new QTimer();
 	connect(stepTimer, SIGNAL(timeout()), this, SLOT(step()));
	init();
	exec();
}

QVariant Module::value(QString ID){return state->value(ID);}
bool Module::boolValue(QString ID){return state->value(ID).toBool();}
int Module::intValue(QString ID){return state->value(ID).toInt();}
double Module::doubleValue(QString ID){return state->value(ID).toDouble();}
QString Module::stringValue(QString ID){return state->value(ID).toString();}
bool Module::available(QString ID){return state->available(ID);}
QTime Module::timestamp(QString ID){return state->timestamp(ID);}

QVariant Module::config(QString ID){
	return settings.value(moduleName+"/"+ID);
}



