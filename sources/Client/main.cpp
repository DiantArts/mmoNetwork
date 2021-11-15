#include <pch.hpp>
#include <Client/ClientExample.hpp>

enum class Enum{ nothing };


int main(int argc, char **argv)
{
    ::ClientExample client;
    ::std::thread thread;

    try {
        if (!client.startConnectingToServer(argv[1], ::std::atoi(argv[2]))) {
            return EXIT_FAILURE;
        }

        thread = ::std::thread{
            [&client](){
                ::std::size_t i{ 0 };
                while (client.isConnectedToServer()) {
                    client.blockingPullIncommingMessages();
                }
            }
        };

        ::std::string str;
        while (client.isConnected()) {
            ::std::getline(::std::cin, str);
            if (!::std::strncmp(str.c_str(), "/", 1)) {
                switch (*str.substr(1, 2).c_str()) {
                case 'q': client.disconnect(); break;
                case 'c': client.startCall(::std::atoi(str.substr(3).c_str())); break;
                case 'u': client.messagePeer(str.substr(3)); break;
                case 's': client.stopCall(); break;
                default: ::std::cerr << "[ERROR:SYSTEM] invalid command.\n";
                }
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
