#include <pch.hpp>
#include <Server/ServerExample.hpp>



int main(int argc, char **argv)
{
    try {

        if (argc != 2) {
            std::cerr << "Usage: client <host>" << std::endl;
            return EXIT_FAILURE;
        }
        ::ServerExample server{ 60000 };
        if (!server.start()) {
            return EXIT_FAILURE;
        }

        while (server.isRunning()) {
            server.blockingPullIncommingMessages();
        }
        return EXIT_SUCCESS;

    } catch (const ::std::exception& e) {
       ::std::cerr << "ERROR: " << e.what() <<::std::endl;
        return EXIT_FAILURE;

    } catch (...) {
       ::std::cerr << "ERROR: unknown" <<::std::endl;
        return EXIT_FAILURE;

    }
}
