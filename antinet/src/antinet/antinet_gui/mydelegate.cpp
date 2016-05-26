#include <QSpinBox>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
//#include
#include <QDebug>


#include "mydelegate.h"

myDelegate::myDelegate()
{
 qDebug()<< "delegate constructor";
}

QWidget *myDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index)
{
//	QSpinBox *editor = new QSpinBox(parent);
//	editor->setMinimum(0);
//	editor->setMaximum(100);

	qDebug()<< "creating delegate";
	QWidget *editor = new QWidget(parent);


//	QLayout *layout = new QLayout(editor);

//		editor->setLayout(layout);

	if(index.column() ==0 ){
		QPushButton *yes_btn  = new QPushButton(parent);
//		QPushButton *no_btn  = new QPushButton(parent);
//		layout->addWidget(yes_btn);
//		layout->addWidget(no_btn);
		editor = yes_btn;
	}
	return editor;

}

void myDelegate::setEditorData(QWidget *editor, const QModelIndex &index)
{

	qDebug()<< "set delegate";
//	int value = index.model()->data(index, Qt::EditRole).toInt();
	if(index.column() != 0 ) {
//		QString value = index.data().toString();
		QStyledItemDelegate::setEditorData(editor,index);
//		static_cast <QLabel*>(editor->children().at(0))->setText(value);
	}
//	QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
//	spinBox->setValue(value);

}

void myDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index)
{
		qDebug()<<"myDelegate";
	QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
	spinBox->interpretText();
	int value = spinBox->value();

	model->setData(index, value, Qt::EditRole);

}

void myDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index)
{
		qDebug()<<"myDelegate";
	editor->setGeometry(option.rect);
}
