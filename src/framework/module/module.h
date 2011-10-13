/**
 * module.h - standardized interface for AUVC modules.
 * Each module has input signals and output signals. A module can subscribe to certain signals, which become its inputs, and may update certain signals, which become its outputs.
 * Settings are stored in ....
 */
#ifndef MODULE_H
#define MODULE_H

#include "../state/state.h"
#include <QThread>
#include <QMap>
#include <QString>
#include <QTimer>

class Module : public QThread
{
	Q_OBJECT
	public:
		/**
		 * Module constructor.
		 * When you implement a constructor in your Module, you must call the parent constructor in the initialization list.
		 * That is, your constructor implementation should look like: MyModule():Module(){
		 */
		//Module(AUVC_State_Data* stateIn = 0);
		Module();
		/**
		 * Subscribe to data updates from the state database by returning
		 * a QStringList with the data IDs you want from here.
		 * Default implementation subscribes to nothing
		 */
		virtual QStringList subscriptions(){return QStringList();}

		/** 
		 * Return a list of data IDs that this module provides to the hub.
		 */
		virtual QStringList provides(){return QStringList();}

		/**
		 * Return true from this function if you want a module to run in its own thread.
		 * Return false and this module will run in the main thread. DO NOT BUSY-WAIT ANY THREAD.
		 */
		virtual bool isThread(){return true;}


	protected slots:
		/**
		 * Inputs - dataIn(VDatum) is called with data updates from moduleHub.
		 * This slot will only be called with VDatums that this Module has subscribed to via the subscriptions() list.
		 * \param datum data data
		 */
		virtual void dataIn(VDatum datum){}

		/**
		 * init() - implement this function to do any work when the application starts.
		 */
		virtual void init(){}

		/**
		 * step() - implement this function to do any work periodically.
		 * Call stepTimer->start([stepTimeInMs]); in your constructor to activate.
		 */
		virtual void step(){}

		/**
		 * Outputs - Data sender.
		 * Data sent via this method will be propogated and broadcasted as far as possible via moduleHub (state).
		 * Once data is set via these methods, it is accessible via the value(QString), intValue(QString), etc. set of functions.
		 * \param id data Identifier
		 * \param value data Value
		 */
		void setData(QString ID, QVariant value, QTime timestamp = QTime::currentTime(), QVariant meta = QVariant());
		/**
		 * Outputs - Data sender.
		 * Data sent via this method will be propogated and broadcasted as far as possible via moduleHub (state).
		 * Once data is set via these methods, it is accessible via the value(QString), intValue(QString), etc. set of functions.
		 * \param id data Identifier
		 * \param value data Value
		 */
		void setData(VDatum);
		/**
		 * Outputs - Data sender.
		 * Data sent via this method will be propogated and broadcasted as far as possible via moduleHub (state).
		 * Once data is set via these methods, it is accessible via the value(QString), intValue(QString), etc. set of functions.
		 * \param id data Identifier
		 * \param value data Value
		 */
		void setData(QList<VDatum>);
		
		/**
		 * Store a configuration value.
		 * Does not overwrite stored values.
		 * Call from init().
		 */
		void setConfig(QString ID, QVariant configData);

	protected:

		/**
		 * Read value from state data.
		 */
		QVariant value(QString ID);
		/**
		 * Read value from state data as boolean type.
		 */
		bool boolValue(QString ID);
		/**
		 * Read value from state data as int type.
		 */
		int intValue(QString ID);
		/**
		 * Read value from state data as double type.
		 */
		double doubleValue(QString ID);
		/**
		 * Read value from state data.
		 */
		QString stringValue(QString ID);
		/**
		 * Read availability from state data.
		 */
		bool available(QString ID);
		/**
		 * Read timestamp from state data.
		 */
		QTime timestamp(QString ID);

		// config variable 
		/**
		 * Read value from configuration file.
		 */
		QVariant config(QString ID);

		// timer for step function
		QTimer* stepTimer;

		bool debug;



	/* Ignore everything below this line ******************************************************************/
	public:
		void setLinks(AUVC_State_Data* state, bool debug);
	private slots:
		void run();
	private:
		AUVC_State_Data* state;
		QSettings settings;
		QMetaObject* meta;
		QString moduleName;
	public slots:
		// internally converted to dataIn(VDatum)
		void recvDatum(QString module, VDatum datum);
	signals:
		// internally converted from setData(QString...
		void sendData(VDatum datum);
};


#endif
