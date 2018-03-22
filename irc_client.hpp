#ifndef IRC_CLIENT_HPP
#define IRC_CLIENT_HPP

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace kq::irc
{
struct settings
{
    std::string host;
    int port;
    std::string nick;
};

class client
{
public:
    using message_handler =
        std::function<
            void(
                std::string_view,
                std::string_view,
                std::string_view
            )
        >;

    client(
        asio::io_context&,
        irc::settings const&
    );

    void join(std::string_view);

    void say(
        std::string_view receiver,
        std::string_view message
    );

    void send(std::string_view);

    void register_handler(
        std::string_view name,
        message_handler handler
    );

    void register_on_connect(
        std::function<void()>
    );

private:

    void connect();

    void identify();

    void on_hostname_resolved(
        boost::system::error_code const&,
        tcp::resolver::results_type
    );

    void on_connected(
        boost::system::error_code const&
    );

    void await_new_line();

    void on_new_line(
        boost::system::error_code const&,
        std::size_t
    );

    void handle_message(
        std::string_view who,
        std::string const& type,
        std::string_view where,
        std::string_view message
    );

    asio::io_context& ctx;
    irc::settings settings;
    tcp::socket socket;
    asio::streambuf in_buf;
    std::unordered_map<
        std::string,
        std::vector<message_handler>
    > handlers;
    std::vector<std::function<void()>> on_connect_handlers;
};

}


#endif // IRC_CLIENT_HPP
