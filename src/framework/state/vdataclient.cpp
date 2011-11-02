#include "vdataclient.h"
#include <QDebug>

VDataClient::VDataClient(){

	if(!settings.contains("VDataServer/bindPort")) settings.setValue("VDataServer/bindPort", 5325);

	if(!m_Sock.bind(QHostAddress::Any, settings.value("VDataServer/bindPort"), QUdpSocket::ShareAddress)) qDebug() << "Failed to bind to port" << settings.value("VDataServer/bindPort");
	else  qDebug() << "Sucessful bind to port" << settings.value("VDataServer/bindPort").toString() << ":" << bindPort;

	m_remoteAddr = remoteAddr;
	m_remotePort = remotePort;
	m_buffer = false;
	m_flaky = false;
	QObject::connect(&m_Sock, SIGNAL(readyRead()),	this, SLOT(handlePendingDatagrams()));
	m_AckTimeout = 2000;

//	if(config.isEmpty()) loadConfigFile(config);
	if(!m_Server){
		sendDatagram("Connect", true);
	}
}

VDataClient::~VDataClient() {
}

void VDataClient::setRemoteAddr(QString addr, quint16 port){
	if(!QHostAddress(addr).isNull()) m_remoteAddr = addr;
	if(port != 0) m_remotePort = port;
	sendDatagram("Connect", true);
}

void VDataClient::sendVDatum(VDatum message, bool critical) {
	QByteArray serializedVDatum;
	// do not send if over 65000 bytes
	serializedVDatum.append("VDatum");
	serializedVDatum.append(serializeVDatum(message));
	if(serializedVDatum.size() < 65000)
		m_outBuffer.append(serializedVDatum);
	else qDebug() << "VDatum exceeds size limit for network transmission:" << message.id;
	if(!m_buffer) {
		sendDatagram(m_outBuffer, critical);
		m_outBuffer.clear();
	}
}

// sends data and checks for acks
void VDataClient::sendDatagram(QByteArray out, bool force) {

	if(m_remoteAddr.isNull()) return;

	if(force){
		// skip error checking
		if(out == "Connect") qDebug() << "Connecting...";
	}else if(m_outQueue.size() > 10){
		qDebug() << "Queue Overflow";
		emit remoteNotResponding();
		return;
	}else if(m_flaky){
		qDebug() << "Still Flaky";
		m_outQueue.enqueue(out);
		QTimer::singleShot(500,this,SLOT(sendDatagram())); 
		return;
	}else if(m_Acks.size() > 5){
		qDebug() << "Acks overflow";
		//qDebug() << QString::number(m_Acks.size()) + " commands still unacknowledged";
		m_flaky=true;
		timeLostConn = QTime::currentTime();
		// the first time we have unacked packets, send a connect datagram
		sendDatagram("Connect", true);
		emit reconnecting();
		return;
	}

	m_Sock.writeDatagram(out, m_remoteAddr, m_remotePort);
	//if(config["Debug"] == "true") 
	if(out.startsWith("VDatum")){
		//qDebug() << "Sent VDatum to " + m_remoteAddr.toString() + ":" + QString::number(m_remotePort);
	}else{
		qDebug() << "Sent " << out << " to " + m_remoteAddr.toString() + ":" + QString::number(m_remotePort);
	}

	m_Acks.insert(out, QTime::currentTime());
	// remove acks more than AckTimeout old and let everyone know what went missing
	foreach(QByteArray datagram, m_Acks.keys()){
		if(m_Acks[datagram].msecsTo(QTime::currentTime()) > m_AckTimeout){
			emit(noAck(datagram));
			m_Acks.remove(datagram);
		}
	}
}

// called by dashboard
void VDataClient::sync(){
	sendDatagram("Update:All", true);
}
void VDataClient::sync(QTime last){
	timeLostConn = last;
	QByteArray updateDat = "Update:";
	updateDat.append(timeLostConn.toString("hh:mm:ss.zzz"));
	sendDatagram(updateDat);
}

void VDataClient::reconnect(){
	timeLostConn = QTime::currentTime();
	sendDatagram("Connect", true);
}

void VDataClient::setAckTimeout(int msecs){
	m_AckTimeout = msecs;
}

void VDataClient::sendDatagram(){
	if(!m_outQueue.isEmpty()) sendDatagram(m_outQueue.dequeue());
}


void VDataClient::handlePendingDatagrams() {
	static QByteArray datagram;
	static QHostAddress sender;
	static quint16 senderPort;
	while (m_Sock.hasPendingDatagrams()) {
		//qDebug() << "Reading Datagram";

		datagram.resize(m_Sock.pendingDatagramSize()); // prep buffer

		m_Sock.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		// TODO - start in thread
		processDatagram(datagram, sender, senderPort);

	}
}

void VDataClient::processDatagram(QByteArray datagram, QHostAddress fromAddr, quint16 fromPort){
	if(m_Acks.contains(datagram)){ // check to see if incoming is an acknowledgement
		if(datagram == "Connect") {
			m_remoteAddr = sender;
			qDebug() << "Connected to " << sender;
			m_flaky = false;
			while(!m_outQueue.isEmpty()) sendDatagram();
			QByteArray updateDat = "Update:";
			updateDat.append(timeLostConn.toString("hh:mm:ss.zzz"));
			sendDatagram(updateDat);
		}
		m_Acks.remove(datagram); // and remove from unanswered list 
	}else{
		qDebug() << "Got Datagram:" << datagram;

		if(!datagram.startsWith("VDatum")) return;
		datagram = datagram.right(datagram.size()-6);

		QList<VDatum> receivedMessage;
		//qDebug() << "Parsing Datagram:" << datagram;
		receivedMessage = parseVDatums(datagram);

		if(fromPort){}
		foreach (VDatum msg, receivedMessage) {
			emit datumReceived(msg, fromAddr);
			//qDebug() << "VDataClient emitting VDatum:" << msg.id;
		}
	}
}

void VDataClient::buffer(){
	m_buffer = true;
}

void VDataClient::flush(){
	m_buffer = false;
	sendDatagram(m_outBuffer);
	m_outBuffer.clear();
}
