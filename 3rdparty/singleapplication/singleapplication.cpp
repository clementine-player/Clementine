#include "singleapplication.h"
#include <cstdlib>

/**
 * @brief Constructor. Checks and fires up LocalServer or closes the program
 * if another instance already exists
 * @param argc
 * @param argv
 */
SingleApplication::SingleApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    QString serverName = QApplication::organizationName() + QApplication::applicationName();
    serverName.replace(QRegExp("[^\\w\\-. ]"), "");

    // Attempt to connect to the LocalServer
    socket = new QLocalSocket();
    socket->connectToServer(serverName);
    if(socket->waitForConnected(1000)){
        socket->close();
        ::exit(EXIT_SUCCESS); // Terminate the program using STDLib's exit function
    } else {
        // If the connection is insuccessful, this is the main process
        // So we create a Local Server
        server = new QLocalServer();
        server->removeServer(serverName);
        server->listen(serverName);
        QObject::connect(server, SIGNAL(newConnection()), this, SLOT(slotConnectionEstablished()));
    }
}

/**
 * @brief Destructor
 */
SingleApplication::~SingleApplication()
{
    server->close();
}

/**
 * @brief Executed when the showUp command is sent to LocalServer
 */
void SingleApplication::slotConnectionEstablished()
{
    server->nextPendingConnection();
    emit showUp();
}

