#include "c_antinet_agent_api.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    c_antinet_agent_api w;
    w.show();

    return a.exec();
}

