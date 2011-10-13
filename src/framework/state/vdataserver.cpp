#include "vdataserver.h"
#include <QDebug>

VDataServer::VDataServer(){
	// load ports from settings

	if(!settings.contains("VDataServer/bindPort")) settings.setValue("VDataServer/bindPort", 5325);


	if(!m_Sock.bind(QHostAddress::Any, settings.value("VDataServer/bindPort"), QUdpSocket::ShareAddress)) qDebug() << "Failed to bind to port" << settings.value("VDataServer/bindPort");
	else  qDebug() << "Sucessful bind to port" << settings.value("VDataServer/bindPort").toString() << ":" << bindPort;
	m_buffer = false;
	QObject::connect(&m_Sock, SIGNAL(readyRead()),
			this, SLOT(handlePendingDatagrams()));
	m_AckTimeout = 2000;

}

VDataServer::~VDataServer() {
}


void VDataServer::sendVDatum(VDatum message, QHostAddress client, bool critical) {
	m_outBuffer.append("VDatum");
	m_outBuffer.append(serializeVDatum(message));
	if(!m_buffer) {
		sendDatagram(m_outBuffer, client, critical);
		m_outBuffer.clear();
	}
}

// sends data and checks for acks
void VDataServer::sendDatagram(QByteArray out, Client client, bool force) {

	if(!client.isNull()){
		m_Sock.writeDatagram(out, client.address, client.port);
	}else{
		foreach(Client cli, clients){
			m_Sock.writeDatagram(out, cli.address, cli.port);
		}
	}

}



void VDataServer::handlePendingDatagrams() {
	static QByteArray datagram;
	static QHostAddress sender;
	static quint16 senderPort;
	while (m_Sock.hasPendingDatagrams()) {
		//qDebug() << "Reading Datagram";

		datagram.resize(m_Sock.pendingDatagramSize()); // prep buffer

		m_Sock.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		// server echos to acknowledge received
		m_Sock.writeDatagram(datagram, sender, senderPort);

		// start in thread
		QtConcurrent::run(this, &VDataServer::processDatagram, datagram, sender, senderPort);

	}
}

void VDataServer::processDatagram(QByteArray datagram, QHostAddress fromAddr, quint16 fromPort){
	qDebug() << "Got Datagram:" << datagram;

	Client cli;
	cli.address = fromAddr;
	cli.port = fromPort;

	if(datagram == "Connect") {
		qDebug() << "Connection from:" << sender;
		if(!clients.contains(cli))
			clients.append(cli);
	} 
	else if(datagram.startsWith("Update:")) {
		qDebug() << "Synchronizing";
		datagram = datagram.right(datagram.size()-7);
		if(datagram == "All"){
			emit syncAllData(cli);
		}else{
			timeLostConn = QTime::fromString(datagram, "hh:mm:ss.zzz");
			emit syncDataFrom(timeLostConn, cli);
		}
	} 
	else if(datagram.startsWith("VDatum")) {
		datagram = datagram.right(datagram.size()-6);

		QList<VDatum> receivedMessage;
		//qDebug() << "Parsing Datagram:" << datagram;
		receivedMessage = parseVDatums(datagram);

		if(fromPort){}
		foreach (VDatum msg, receivedMessage) {
			emit datumReceived(msg, fromAddr);
			//qDebug() << "VDataServer emitting VDatum:" << msg.id;
		}
	}
}

void VDataServer::buffer(){
	m_buffer = true;
}

void VDataServer::flush(){
	m_buffer = false;
	sendDatagram(m_outBuffer);
	m_outBuffer.clear();
}
