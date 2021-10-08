#include <pch.hpp>
#include <Client/ClientExample.hpp>

int main(int argc, char **argv)
{
    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>" << std::endl;
            return EXIT_FAILURE;
        }
        ::ClientExample client;
        if (!client.connect("127.0.0.1", 60000)) {
            return EXIT_FAILURE;
        }

        ::std::thread thread{
            [&client](){
                ::std::size_t i{ 0 };
                while (client.isConnected()) {
                    client.getIncommingMessages().wait();
                    client.handleMessagesIn();
                }
            }
        };

        ::std::string str;
        while (client.isConnected()) {
            ::std::getline(::std::cin, str);
            if (str == "/q") {
                client.disconnect();
            } else if (str == "/p") {
                client.pingServer();
            } else {
                client.messageServer();
                // this->send<::packet::Text>(str);
            }
        }
        client.stop();
        client.disconnect();
        thread.join();

        return EXIT_SUCCESS;

    } catch (const ::std::exception& e) {
       ::std::cerr << "ERROR: " << e.what() <<::std::endl;
        return EXIT_FAILURE;

    } catch (...) {
       ::std::cerr << "ERROR: unknown" <<::std::endl;
        return EXIT_FAILURE;

    }
}
