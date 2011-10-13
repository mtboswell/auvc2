/**
 * @file vdatasocket.h
 * Class provides a VDatum (Simple Id/Data Format) Network Socket
 */
#ifndef __VDataServer_H
#define __VDataServer_H

#include "vdatum.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QHash>
#include <QStringList>

struct Client {
	QHostAddress address;
	quint16 port;
};

class VDataServer : public QObject {
	Q_OBJECT
	public:
		/**
		 * Constructor.
		 * @param bindPort local port to bind to
		 * @param remotePort remote port to talk to
		 * @param remoteAddr remote host
		 * @param bindAddr local address to bind to
		 */
		VDataServer();
		virtual ~VDataServer();

	signals:
		/**
		 * Received a VData packet.
		 */
		//void tmfReceived(QString ID, QString data);
		//void dataReceived(VData message, QHostAddress from);
		void datumReceived(VDatum message, QHostAddress from);

		/**
		 * Signal to send any data from time when connection was lost
		 */
		void connectionRestored(QTime timeLost, QHostAddress client);
		
		/// signal to re-broadcast all data
		void syncData(QHostAddress client);

	public slots:
		/** 
		 * Send data to other host.
		 * @param ID data identifier
		 * @param data data
		 */
		void sendVDatum(VDatum message, QHostAddress client = QHostAddress(), bool critical=false);

		// hold outgoing messages until flush() is called
		void buffer();
		// release/send buffered outgoing messages
		void flush();

	private slots:
		void sendDatagram(QByteArray out, bool resend = false);
		void sendDatagram();
		void handlePendingDatagrams();
		void processDatagram(QByteArray, QHostAddress, quint16);

	private:
		QList<Client> clients;
		QUdpSocket m_Sock;
		QHostAddress m_remoteAddr;
		QHostAddress m_localAddr;
		quint16 m_remotePort;
		quint16 m_localPort;
		QHash<QByteArray,QTime> m_Acks;
		bool m_buffer;
		QQueue<QByteArray> m_outQueue;
		QByteArray m_outBuffer;
		QMap<QString, QString> config;
		QTime timeLostConn;
};

#endif //#ifndef __VDataSOCKET_H
