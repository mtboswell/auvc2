/****************************************************************************
 **
 ** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include <QtGui>
#include <QDebug>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel(QObject *parent)
: QAbstractItemModel(parent)
{
	QVector<QVariant> rootData;
	QStringList headers;
	headers << "ID" << "Value" << "Timestamp" << "Availabile" << "Meta" << "Path";
	foreach (QString header, headers)
		rootData << header;

	rootItem = new TreeItem(rootData);
	/*
	rootItem->insertChildren(rootItem->childCount(), 1, rootItem->columnCount());
	TreeItem* newItem = rootItem->child(rootItem->childCount() - 1);
	newItem->setData(0, "Testing!");
	setData("Testing_Param", "Hello");
	*/
}

TreeModel::~TreeModel()
{
	delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
	return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	TreeItem *item = getItem(index);

	return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	if(index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
	if (index.isValid()) {
		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		if (item) return item;
	}
	return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	TreeItem *parentItem = getItem(parent);

	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
	bool success;

	beginInsertColumns(parent, position, position + columns - 1);
	success = rootItem->insertColumns(position, columns);
	endInsertColumns();

	return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	TreeItem *parentItem = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, rootItem->columnCount());
	endInsertRows();

	return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = getItem(index);
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
	bool success;

	beginRemoveColumns(parent, position, position + columns - 1);
	success = rootItem->removeColumns(position, columns);
	endRemoveColumns();

	if (rootItem->columnCount() == 0)
		removeRows(0, rowCount());

	return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	TreeItem *parentItem = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem = getItem(parent);

	return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role != Qt::EditRole)
		return false;

	TreeItem *item = getItem(index);
	bool result = item->setData(index.column(), value);

	if (result){
		if(index.column() == 1){
			item->setData(2, QTime::currentTime());
			item->setData(3, true);
		}	
		emit dataChanged(index, index);

		QString itemName = item->data(0).toString();
		QString itemParents = "";
		TreeItem* iParent = item->parent();
		while(iParent != rootItem){
			itemParents.prepend(iParent->data(0).toString() + ".");
			itemName.prepend(iParent->data(0).toString() + ".");
			iParent = iParent->parent();
		}
		emit dataUpdated(itemName, item->data(1));
		VDatum tmp;
		tmp.id = item->data(0).toString().prepend(itemParents);
		tmp.value = item->data(1);
		tmp.timestamp = item->data(2).toTime();
		tmp.available = item->data(3).toBool();
		tmp.meta = item->data(4);
		emit dataUpdated(tmp);
		emit dataEdited(tmp);
	}

	return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
		const QVariant &value, int role)
{
	if (role != Qt::EditRole || orientation != Qt::Horizontal)
		return false;

	bool result = rootItem->setData(section, value);

	if (result)
		emit headerDataChanged(orientation, section, section);

	return result;
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
	foreach(QString line, lines){
		// parse lines
		//qDebug() << "Parsing:" << line;
		QStringList sids = line.split(";");
		sids.removeAll("");
		foreach(QString sid, sids){
			// parse sids
			QStringList SIDstuff = sid.split("=");
			SIDstuff.removeAll("");
			//qDebug() << "Found SID:" << SIDstuff;
			if(SIDstuff.size() < 2) continue;
			QString SIDid = SIDstuff[0];
			QVariant SIDdata;
			bool isDouble = false;
			double doubleDataValue = SIDstuff[1].toDouble(&isDouble);
			if(isDouble)
				SIDdata = doubleDataValue;
			else
				SIDdata = SIDstuff[1];

			//qDebug() << "New Item:" << SIDid << SIDdata;
			TreeItem* param = getItem(SIDid, parent);
			param->setData(1, SIDdata);
		}
	}


}


TreeItem* TreeModel::getItem(QString id){
	TreeItem* thisItem = getItem(id, rootItem);
	if(thisItem == rootItem){
		qDebug() << "Item location/creation error";
		return thisItem;
	}
	if(!(*thisItem)[5].isValid())
		thisItem->setData(5, id);
	return thisItem;
}

// returns the item for the specified SID ID (e.g. Parameter.Actor.YawController)
TreeItem* TreeModel::getItem(QString value, TreeItem* parentItem){
	QString level_delimiter = ".";
	QStringList thisIDTree = value.split(level_delimiter);
	thisIDTree.removeAll("");
	if(thisIDTree.isEmpty())
		return parentItem;
	QString nameToLookFor = thisIDTree.takeFirst();
	// look in children for item
	for(int i = parentItem->childCount()-1; i >= 0; --i){
		TreeItem* thisItem = parentItem->child(i);
		QVariant thisItemName = thisItem->data(0);
		if(thisItemName == nameToLookFor){
			if(thisIDTree.isEmpty())
				return thisItem;
			else
				return getItem(thisIDTree.join(level_delimiter), thisItem);
		}
	}
	// item not found, create it
	parentItem->insertChildren(parentItem->childCount(), 1, rootItem->columnCount());
	TreeItem* newItem = parentItem->child(parentItem->childCount() - 1);
	newItem->setData(0, nameToLookFor);
	newItem->setData(2, QTime());
	newItem->setData(3, false);

	if(thisIDTree.isEmpty())
		return newItem;
	else
		return getItem(thisIDTree.join(level_delimiter), newItem);

}

void TreeModel::setData(VDatum datum){
	setData(datum.id, datum.value, datum.timestamp, datum.available, datum.meta);
}
void TreeModel::setData(QString id, QVariant value, QTime timestamp, bool available, QVariant meta){
	TreeItem* thisItem = getItem(id, rootItem);
	if(thisItem == rootItem){
		qDebug() << "Item location/creation error";
		return;
	}
	thisItem->setData(1, value);
	thisItem->setData(2, timestamp);
	thisItem->setData(3, available);
	thisItem->setData(4, meta);
	thisItem->setData(5, id);
	QModelIndex rootIndex = index(0,0);
	QModelIndex thisIndex = getIndex(thisItem, 3);
	emit dataChanged(thisIndex, thisIndex);
	emit layoutChanged();
	emit dataUpdated(id, thisItem->data(1));
	//qDebug() << "Module setting " << id << "to" << value;
}

QModelIndex TreeModel::getIndex(TreeItem* thisItem, int column){
	return createIndex(thisItem->childNumber(), column, thisItem);
/*
	QStack<TreeItem*> parents;
	TreeItem* iParent = thisItem;
	while(iParent != rootItem){
		iParent = iParent->parent();
		parents << iParent;
	}
	QModelIndex thisIndex;
	while(!parents.isEmpty()){
		TreeItem* iItem = parents.pop();
		thisIndex = createIndex(iItem->childNumber(), 0, thisIndex);
	}
	QModelIndex
*/
}


QVariant TreeModel::value(QString ID){
	return (*getItem(ID))[1];
}
QTime TreeModel::timestamp(QString ID){
	return (*getItem(ID))[2].toTime();
}

bool TreeModel::available(QString ID){
	return (*getItem(ID))[3].toBool();
}

void TreeModel::sync(QTime last){
	sync(last, rootItem);
}
// this function contains a stack overflow bug
void TreeModel::sync(QTime last, TreeItem* parentItem){
	if(parentItem == 0) parentItem = rootItem;
	if(parentItem == rootItem) qDebug() << "Syncing";
	// foreach treeitems
	// 	if time > last
	// 		emit syncData(item->vdatum)
	TreeItem* thisItem;
	QTime thisItemDate;
	for(int i = parentItem->childCount()-1; i >= 0; --i){
		thisItem = parentItem->child(i);
		if(thisItem->childCount() > 0){
			sync(last, thisItem);
		}else{
			thisItemDate = thisItem->data(2).toTime();
			if(thisItemDate > last){
				emit syncData(thisItem->toVDatum());
			}
		}
	}
}
void TreeModel::sync(){
	sync(rootItem);
}
// this function contains a stack overflow bug
void TreeModel::sync(TreeItem* parentItem){
	if(parentItem == 0) parentItem = rootItem;
	if(parentItem == rootItem) qDebug() << "Syncing";
	// foreach treeitems
	// 	emit syncData(item->vdatum)
	TreeItem* thisItem;
	for(int i = parentItem->childCount()-1; i >= 0; --i){
		thisItem = parentItem->child(i);
		if(thisItem->childCount() > 0){
			sync(thisItem);
		}else{
			emit syncData(thisItem->toVDatum());
		}
	}
}

