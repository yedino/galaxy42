#include "datamodel.h"

dataModel::dataModel(QObject *parent)
	: QAbstractItemModel(parent)
{
}

QVariant dataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// FIXME: Implement me!
}

QModelIndex dataModel::index(int row, int column, const QModelIndex &parent) const
{
	// FIXME: Implement me!
}

QModelIndex dataModel::parent(const QModelIndex &index) const
{
	// FIXME: Implement me!
}

int dataModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
		return 0;

	// FIXME: Implement me!
}

int dataModel::columnCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
		return 0;

	// FIXME: Implement me!
}

QVariant dataModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	// FIXME: Implement me!
	return QVariant();
}
