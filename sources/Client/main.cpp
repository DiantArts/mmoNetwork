#include <pch.hpp>
#include <Client/ClientExample.hpp>



int main(int argc, char **argv)
{
    if (argc != 3) {
        ::std::cerr << "Usage: client <host> <port>" << ::std::endl;
        return EXIT_FAILURE;
    }

    ::ClientExample client;
    ::std::thread thread;

    try {
        if (!client.connectToServer(argv[1], ::std::atoi(argv[2]))) {
            return EXIT_FAILURE;
        }

        thread = ::std::thread{
            [&client](){
                ::std::size_t i{ 0 };
                while (client.isConnectedToServer()) {
                    client.getIncommingMessages().wait();
                    client.handleMessagesIn();
                }
            }
        };

        ::std::string str;
        while (client.isConnected()) {
            ::std::getline(::std::cin, str);
            if (!::std::strncmp(str.c_str(), "/q", 2)) {
                client.disconnect();
            } else if (!::std::strncmp(str.c_str(), "/p", 2)) {
                client.pingServer();
            } else if (!::std::strncmp(str.c_str(), "/c", 2)) {
                client.startCall(::std::atoi(str.substr(3).c_str()));
            } else if (!::std::strncmp(str.c_str(), "/u", 2)) {
                client.messagePeer(str.substr(3));
            } else {
                client.messageServer(str);
            }
        }

        client.getIncommingMessages().notify();
        thread.join();
        return EXIT_SUCCESS;

    } catch (const ::std::exception& e) {
       ::std::cerr << "ERROR: " << e.what() <<::std::endl;
        client.disconnect();
        client.getIncommingMessages().notify();
        thread.join();
        return EXIT_FAILURE;

    } catch (...) {
       ::std::cerr << "ERROR: unknown" <<::std::endl;
        client.disconnect();
        client.getIncommingMessages().notify();
        thread.join();
        return EXIT_FAILURE;

    }

}
