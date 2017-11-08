#ifndef TEST_MESHPEER_H
#define TEST_MESHPEER_H

#include <json.hpp>
#include <QtTest/QtTest>

#include "meshpeer.h"

using nlohmann::json;

class Test_Meshpeer:public QObject
{
    Q_OBJECT

    MeshPeer m_test_obj;
    std::string m_serilized_obj;

private slots:
    void serialize_test()
    {
        try{
            m_test_obj.setName("myPeer");
            m_test_obj.setIP("127.223.42.22");
            m_test_obj.setVip("fdaee:1223dfa");
        }catch (std::runtime_error &e){
          QVERIFY(false);
        }
        m_serilized_obj = m_test_obj.serialize();
        json j(m_serilized_obj);

        QVERIFY (m_test_obj.getName() == QString::fromStdString(j["name"]));
        QVERIFY (m_test_obj.getIp() == QString::fromStdString(j["ip"]));
        QVERIFY (m_test_obj.getVip() == QString::fromStdString(j["vip"]));
    }

    void deserialize_test()
    {
        try {

            MeshPeer t_obj(m_serilized_obj);
            QVERIFY(t_obj.getName() == m_test_obj.getName());
            QVERIFY(t_obj.getIp() == m_test_obj.getIp());
            QVERIFY(t_obj.getVip() == m_test_obj.getVip());

        } catch (std::runtime_error &e) {
            QVERIFY(false);
        }
    }
};


#endif // TEST_MESHPEER_H
