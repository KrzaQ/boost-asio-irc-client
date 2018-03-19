#include <iostream>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std::literals;

namespace asio = boost::asio;

int main()
{
    asio::io_context io;
    io.run();
}
