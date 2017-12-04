#include <QStyledItemDelegate>
#include "peereditdialog.h"

#ifndef PEERDELEGATE_H
#define PEERDELEGATE_H


class PeerDelegate : public  QStyledItemDelegate
{
    PeerEditDialog *m_dlg;

public:
    PeerDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

#endif // PEERDELEGATE_H
