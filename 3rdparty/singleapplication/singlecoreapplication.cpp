#include "singlecoreapplication.h"
#include <cstdlib>

/**
 * @brief Constructor. Checks and fires up LocalServer or closes the program
 * if another instance already exists
 * @param argc
 * @param argv
 */
SingleCoreApplication::SingleCoreApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv)
{
    QString serverName = QCoreApplication::organizationName() + QCoreApplication::applicationName();
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
SingleCoreApplication::~SingleCoreApplication()
{
    server->close();
}

/**
 * @brief Executed when the showUp command is sent to LocalServer
 */
void SingleCoreApplication::slotConnectionEstablished()
{
    server->nextPendingConnection();
    emit showUp();
}

