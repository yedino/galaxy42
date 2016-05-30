#ifndef MYDELEGATE_H
#define MYDELEGATE_H

#include <QStyledItemDelegate>

class myDelegate : public QStyledItemDelegate
{
public:
	myDelegate();

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index);

	void setEditorData(QWidget *editor, const QModelIndex &index);
	void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index);

	void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index);
};

#endif // MYDELEGATE_H
