#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFileIconProvider>
#include <QAbstractTableModel>
#include <QTcpServer>
//#include <Qt>


class infoModel:public QAbstractTableModel
{
public:

		infoModel(const QStringList &strings, QObject *parent = 0)
		   : QAbstractTableModel(parent), m_stringList(strings) {}

	   int rowCount(const QModelIndex &parent = QModelIndex()) const {return m_stringList.size();}

	   QVariant data(const QModelIndex &index, int role) const {
		QFileIconProvider iconProvider;
	   if (!index.isValid())
			   return QVariant();

		   if (index.row() >= m_stringList.size())
			   return QVariant();

		   if (role == Qt::DisplayRole) {
				if(index.column() == 0){
					return m_stringList.at(index.row());
				}
		   } else if (role == Qt::DecorationRole){
   //             return QIcon("~/icon.png");
				// tu dobor ikony
//                return iconProvider.icon(QFileIconProvider::Computer);
			   if(m_stringList.at(index.row()).contains("error:")){
				return QIcon(":/error.png");
			   }else if(m_stringList.at(index.row()).contains("warrning:")){
				   return QIcon(":/warning.png");
			   }else {
				   return QVariant();
			   }

		   } else if(role == Qt::CheckStateRole) {
			   if(m_stringList.at(index.row()).contains("question"))
			   {
				if( m_stringList.at(index.row()).contains("yes;"))
					return Qt::Checked;
				if( m_stringList.at(index.row()).contains("no;"))
					return Qt::Unchecked;
			   }else
				return QVariant();
//          } else if (role == Qt::EditRole)
		   }else
			   return QVariant();
		}

	   QVariant headerData(int section, Qt::Orientation orientation,
						   int role = Qt::DisplayRole) const
	   {
		   if (role != Qt::DisplayRole)
			   return QVariant();

		   if (orientation == Qt::Horizontal)
			   return QString("Column %1").arg(section);
		   else
			   return QString("Row %1").arg(section);

	   }
	   int columnCount(const QModelIndex &parent) const{return 1;}

 //      void setStringList(&QStringList lst){
 //          m_stringList = lst;
 //      }

	   void addMessage(QString msg){
		   beginInsertRows(QModelIndex(),m_stringList.size()-1,m_stringList.size()-1);
		   m_stringList.push_back(msg);
			endInsertRows();
	   }


	   void clicked( const QModelIndex& index )
	   {


			  if (m_stringList.at(index.row()).contains("yes;")) {
				  m_stringList.replace(index.row(), m_stringList[index.row()].replace("yes;","no;"));
			  } else if(m_stringList.at(index.row()).contains("no;")) {
					m_stringList.replace(index.row() , m_stringList[index.row()].replace("no;","yes;"));
			  }
	   }



   private:
	   QStringList m_stringList;




};

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
	Q_OBJECT

		QTcpServer m_server;
		QTcpSocket *m_tmp_socket;

		infoModel *m_info_model;
public:


	explicit Dialog(QWidget *parent = 0);
	~Dialog();

public slots:
		void onRead();
		void write(QByteArray);
		void onConnect();

private slots:
	void on_qrPushButton_4_clicked();



private:
	Ui::Dialog *ui;
};

#endif // DIALOG_H
