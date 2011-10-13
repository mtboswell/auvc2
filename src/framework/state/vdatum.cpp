#include "vdatum.h"
#include <QDebug>

QList<VDatum> parseVDatums(QByteArray treeData){
	QDataStream in(treeData);
	// legacy name
	QList<VDatum> tree;
/*
	// Read and check the header
	quint32 magic;
	in >> magic;
	if (magic != 0x01212220)
		return QList<VDatum>();
	// Read the version
	qint32 version;
	in >> version;
	if (version != 200)
		return QList<VDatum>();
*/
	in.setVersion(QDataStream::Qt_4_0);

	VDatum thisItem;
	while(!in.atEnd()){
		in >> thisItem.id;
		in >> thisItem.value;
		in >> thisItem.timestamp;
		in >> thisItem.available;
		in >> thisItem.meta;
		tree << thisItem;
	}
	return tree;
}

QByteArray serializeVDatum(VDatum treeItem){
	QByteArray outArray;
	QDataStream out(&outArray,QIODevice::WriteOnly);
	//QDataStream out(&outArray,QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_4_0);

	out << treeItem.id;
	out << treeItem.value;
	out << treeItem.timestamp;
	out << treeItem.available;
	out << treeItem.meta;

	//qDebug() << "Serialized: " << treeItem.id << outArray.size() << out;
	return outArray;
}
// legacy parameter name
QByteArray serializeVDatums(QList<VDatum> tree){
	QByteArray outArray;
	QDataStream out(&outArray,QIODevice::WriteOnly);

	// Write a header with a "magic number" and a version
	// A = 1, U = 21, V = 22, T = 20
	out << (quint32)0x01212220;
	out << (qint32)200;

	out.setVersion(QDataStream::Qt_4_0);

	// Write the data
	//! \todo implement actual hierarchy
	foreach(VDatum item, tree){
		//item.id.prepend(tree.parentID+".");
		out << serializeVDatum(item);
	}
	return outArray;
}
