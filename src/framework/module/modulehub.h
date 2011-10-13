#include <QObject>
#include <QString>
#include <QMetaObject>
#include <QMetaMethod>
#include <QStringList>
#include <QDebug>
#include "../state/state.h"
#include "../state/vdatum.h"
#include "../module/module.h"
#include "../module/simulinkmodule.h"
#include "../state/vdatasocket.h"

/**
 * DataHub is a class designed to connect and manage Modules.
 * \ingroup HUB
 */
class ModuleHub : public QObject
{
	Q_OBJECT
	public:
		ModuleHub(AUVC_State_Data* stateIn, QStringList* debugIn, bool server = true);
		/**
		 * addModule adds a module object to the message broadcasting system.
		 * The module object should have a signal messageOut(QString), and a slot
		 * called messageIn(QString).  Messages are VDatas.
		 * \param module pointer to module QObject
		 */
		void addModule(Module* module);
		void addModule(GuiModule* module);
		/**
		 * As an alternative to the addModule function, you can set this module as the parent of your modules, then call this function.
		 */
		void initializeAndLaunchAllModules();

		void addSubscriptions(QString module, QStringList IDs);
		void addSubscription(QString module, QString ID);

	signals:
		void messageBroadcast(VDatum msg);
		void go(); // used to launch modules
		// subscription outputs
		void subOut(QString, VDatum);

	public slots:
		void messageIn(VDatum msg);
		void moduleIn(VDatum msg);
		void editIn(VDatum msg);
		void startAll(){emit go();}
		void sync();
		void setRemoteAddr(QString addr){srv->setRemoteAddr(addr);}

	private:
		VDataSocket* srv;
		AUVC_State_Data* state;
		QSettings settings;
		QStringList* debug;
		QMap<QString, QStringList> subscriptions;
		void notify(VDatum msg);
};
