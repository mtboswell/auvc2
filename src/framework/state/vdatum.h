#ifndef VData_H
#define VData_H

#include <QStringList>
#include <QMap>
#include <QTime>
#include <QVariant>

// vehicle universal data type
typedef struct VDatum {
	QString id;
	QVariant value;
	QTime timestamp;
	quint8 available;
	QVariant meta;
} VDatum;

QList<VDatum> parseVDatums(QByteArray treeData);

QByteArray serializeVDatum(VDatum datum);
QByteArray serializeVDatums(QList<VDatum> datums);

#endif
