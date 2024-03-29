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

/*
   treeitem.cpp

   A container for items of data supplied by the simple tree model.
 */

#include <QStringList>
#include <QDebug>

#include "treeitem.h"

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
	parentItem = parent;
	itemData = data;
	lock = new QReadWriteLock();
}

TreeItem::~TreeItem()
{
	QWriteLocker locker(lock);
	qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number)
{
	QReadLocker locker(lock);
	return childItems.value(number);
}

int TreeItem::childCount() const
{
	QReadLocker locker(lock);
	return childItems.count();
}

int TreeItem::childNumber() const
{
	QReadLocker locker(lock);
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

int TreeItem::columnCount() const
{
	QReadLocker locker(lock);
	return itemData.count();
}

QVariant TreeItem::data(int column) const
{
	QReadLocker locker(lock);
	return itemData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
	if (position < 0 || position > childItems.size())
		return false;
	QWriteLocker locker(lock);

	for (int row = 0; row < count; ++row) {
		QVector<QVariant> data(columns);
		TreeItem *item = new TreeItem(data, this);
		childItems.insert(position, item);
	}

	return true;
}

bool TreeItem::insertColumns(int position, int columns)
{
	if (position < 0 || position > itemData.size())
		return false;
	QWriteLocker locker(lock);

	for (int column = 0; column < columns; ++column)
		itemData.insert(position, QVariant());

	foreach (TreeItem *child, childItems)
		child->insertColumns(position, columns);

	return true;
}

TreeItem *TreeItem::parent()
{
	return parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
	if (position < 0 || position + count > childItems.size())
		return false;
	QWriteLocker locker(lock);

	for (int row = 0; row < count; ++row)
		delete childItems.takeAt(position);

	return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
	if (position < 0 || position + columns > itemData.size())
		return false;
	QWriteLocker locker(lock);

	for (int column = 0; column < columns; ++column)
		itemData.remove(position);

	foreach (TreeItem *child, childItems)
		child->removeColumns(position, columns);

	return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
	if (column < 0 || column >= itemData.size())
		return false;
	QWriteLocker locker(lock);

	//qDebug() << "Setting data in column" << column << "to" << value;
	itemData[column] = value;
	//if(column==1 || column == 3) itemData[2] = QTime::currentTime();
	return true;
}

QVariant & TreeItem::operator[](int col){
	QReadLocker locker(lock);
	return itemData[col];
}

VDatum TreeItem::toVDatum(){
	QReadLocker locker(lock);
	VDatum out;

	out.id = itemData[5].toString();
	out.value = itemData[1];
	out.timestamp = itemData[2].toTime();
	out.available = itemData[3].toBool();
	out.meta = itemData[4];
	return out;
}
