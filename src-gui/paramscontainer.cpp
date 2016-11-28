#include "paramscontainer.hpp"

#include <QFile>
#include <QDebug>

ParamsContainer::ParamsContainer(QObject *parent) : QObject(parent)
{

}

bool ParamsContainer::readParams(QString file_name)
{
	QFile file(file_name);
	m_was_readed = false;
	if (!file.open(QFile::ReadOnly)){
		return false;
	}
	QByteArray data = file.readAll();
	file.close();

    //QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    //QJsonObject params_obj = loadDoc.object()["peerList"].toObject();
	//for (const auto &it : params_obj.keys())
		//m_peers.insert(it,params_obj[it].toString());
	return true;
}

bool ParamsContainer::writeParams(QString file_name)
{
	QFile file(file_name);
	if (!file.open(QFile::ReadOnly)){
		return false;
	}
//	QJsonObject peersLst;
//	//for (auto it : m_peers.keys())
//	//	peersLst[it] = m_peers[it];

//	QJsonObject doc_obj;
//	doc_obj["peerList"] = peersLst;

//	QJsonDocument doc(doc_obj);

//	file.write(doc.toJson(),doc.toJson().size());
//	file.close();
}

QString ParamsContainer::getIp() {return m_my_ip;}
std::vector<peer_reference> ParamsContainer::getPeerList() {return m_peers;}

void ParamsContainer::setIp(QString my_ip){ m_my_ip = my_ip;}
void ParamsContainer::setPeerList(std::vector<peer_reference> &peer_list) {m_peers = peer_list;}
