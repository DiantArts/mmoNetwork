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
        server.start();
        while (server.isRunning()) {
            ::std::this_thread::sleep_for(50ms);
            server.update();
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
