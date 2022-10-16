#include <pch.hpp>
#include <Client/ClientExample.hpp>

enum class Enum{ nothing };


#include <QApplication>
#include <QLabel>
#include <QLineEdit>

#define FILE fine.hpp
#include <FILE>

int main(int argc, char *argv[])
{
    return 0;
    ::ClientExample client;
    ::std::thread inMessagesThread;

    try {
        if (!client.connectToServer(argv[1], ::std::atoi(argv[2]))) {
            ::std::cerr << "ERROR: Could not connect the following server: "
                << argv[1] << ":" << ::std::atoi(argv[2]) << "." << ::std::endl;
            return EXIT_FAILURE;
        }

        inMessagesThread = ::std::thread{
            [&client](){
                ::std::size_t i{ 0 };
                while (client.isConnectedToServer()) {
                    client.blockingPullIncommingMessages();
                }
            }
        };

        ::std::string str;
        while (true) {
            ::std::getline(::std::cin, str);
            if (!client.isConnected()) {
                break;
            } else if (str.size()) {
                if (!::std::strncmp(str.c_str(), "/", 1)) {
                    switch (*str.substr(1, 2).c_str()) {
                    case 'h': client.commandHelp(); break;
                    case 'q': client.disconnect(); goto ExitWhile;
                    case 'u': client.messageUdpServer(str.substr(3)); break;
                    case 'n': client.rename(str.substr(3)); break;
                    case 'c': client.displayConnectedClients(); break;
                    default: ::std::cerr << "[ERROR:SYSTEM] invalid command.\n";
                    }
                } else {
                    client.messageTcpServer(str);
                }
            }
        }
ExitWhile:

        client.getIncommingMessages().notify();
        inMessagesThread.join();
        return EXIT_SUCCESS;

    } catch (const ::std::exception& e) {
       ::std::cerr << "ERROR: " << e.what() <<::std::endl;
        client.disconnect();
        client.getIncommingMessages().notify();
        inMessagesThread.join();
        return EXIT_FAILURE;

    } catch (...) {
       ::std::cerr << "ERROR: unknown" <<::std::endl;
        client.disconnect();
        client.getIncommingMessages().notify();
        inMessagesThread.join();
        return EXIT_FAILURE;

    }

}
