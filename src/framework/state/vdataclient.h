/**
 * @file vdatasocket.h
 * Class provides a VDatum (Simple Id/Data Format) Network Socket
 */
#ifndef __VDataSOCKET_H
#define __VDataSOCKET_H

#include "vdatum.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QHash>
#include <QStringList>

class VDataSocket : public QObject {
	Q_OBJECT
	public:
		/**
		 * Constructor.
		 * @param bindPort local port to bind to
		 * @param remotePort remote port to talk to
		 * @param remoteAddr remote host
		 * @param bindAddr local address to bind to
		 */
		VDataSocket(quint16 bindPort, quint16 remotePort, bool server = false, QHostAddress remoteAddr = QHostAddress::Null, QHostAddress bindAddr = QHostAddress::Any);
		virtual ~VDataSocket();

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
		void connectionRestored(QTime timeLost);
		/**
		 * May have lost some packets, sent data will be delayed by 500 ms until connection is confirmed.
		 */
		void reconnecting();
		/**
		 * Packet was not acked within AckTimeout msecs..
		 * Use this to deal with re-sending critical data.
		 * Params are last data to be not sent.
		 */
		void noAck(QByteArray missedAck);
		/**
		 * Data is not getting through yet.
		 * Params are last data to be not sent.
		 */
		void remoteNotResponding();
		
		/// signal to re-broadcast all data
		void syncData();

	public slots:
		/** 
		 * Send data to other host.
		 * @param ID data identifier
		 * @param data data
		 */
		void sendVDatum(VDatum message, bool critical=false);
		/**
		 * Send all data to the specified address and port.
		 */
		void setRemoteAddr(QString addr, quint16 port = 0);
		/** 
		 * Set time that we wait for data to be acknowledged on the other end.
		 * If you need low-latency, set this low.
		 * @param msec Duration of timeout, in milliseconds.
		 */
		void setAckTimeout(int msec);

		// hold outgoing messages until flush() is called
		void buffer();
		// release/send buffered outgoing messages
		void flush();
		// trigger remote to send all data
		void sync();
		// ask remote for all data updated since time arg
		void sync(QTime since);
		// attempt to contact remote host
		void reconnect();

	private slots:
		void sendDatagram(QByteArray out, bool resend = false);
		void sendDatagram();
		void handlePendingDatagrams();
		void processDatagram(QByteArray, QHostAddress, quint16);

	private:
		bool m_Server;
		QUdpSocket m_Sock;
		QHostAddress m_remoteAddr;
		QHostAddress m_localAddr;
		quint16 m_remotePort;
		quint16 m_localPort;
		QHash<QByteArray,QTime> m_Acks;
		bool m_flaky;
		bool m_buffer;
		int m_AckTimeout;
		QQueue<QByteArray> m_outQueue;
		QByteArray m_outBuffer;
		QMap<QString, QString> config;
		QTime timeLostConn;
};

#endif //#ifndef __VDataSOCKET_H
