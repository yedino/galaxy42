#include "peerdelegate.h"
#include "peereditdialog.h"
#include "peersmodel.h"


PeerDelegate::PeerDelegate()
{
    m_dlg = new PeerEditDialog;
}

QWidget* PeerDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                                     const QModelIndex &index )
{
    m_dlg = new PeerEditDialog;

    return m_dlg;

}

void PeerDelegate::setEditorData( QWidget *editor, const QModelIndex &index )
{
    QModelIndex *idx = index.sibling( index.row(),peersModel::name );

    QString name = index.model()->data( index );
    idx = index.sibling( index.row(),peersModel::ip );
    QString ip  = index.model()->data( index );
    idx = index.sibling( index.row(),peersModel::vip );
    QString vip = index.model()->data( index );

    dlg->setData( name,vip,ip );

}

void PeerDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index )
{

    m_dlg = static_cast<PeerEditDialog *>( editor );

    QString name = m_dlg->getName();
    QString ip = m_dlg->getIp();
    QString vip = m_dlg->getVip();

    QModelIndex *idx = index.sibling( index.row(),peersModel::name );

    model->setData( index,name );
    idx = index.sibling( index.row(),peersModel::ip );
    model->setData( index,ip );
    idx = index.sibling( index.row(),peersModel::vip );
    model->setData( index,vip );

}

void PeerDelegate::updateEditorGeometry( QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index )
{

}
