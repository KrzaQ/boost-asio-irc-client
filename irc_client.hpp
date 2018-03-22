#ifndef IRC_CLIENT_HPP
#define IRC_CLIENT_HPP

#include <deque>
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
        asio::io_context& ctx,
        irc::settings const& settings
    );

    void join(std::string_view channel);

    void say(
        std::string_view receiver,
        std::string_view message
    );

    void send_raw(std::string data);

    void register_handler(
        std::string_view name,
        message_handler handler
    );

    void register_on_connect(
        std::function<void()> handler
    );

private:

    void connect();

    void identify();

    void on_hostname_resolved(
        boost::system::error_code const& error,
        tcp::resolver::results_type results
    );

    void on_connected(
        boost::system::error_code const& error
    );

    void await_new_line();

    void on_new_line(
        boost::system::error_code const& error,
        std::size_t bytes_read
    );

    void handle_message(
        std::string_view who,
        std::string const& type,
        std::string_view where,
        std::string_view message
    );

    void send_raw_impl();

    void handle_write(
        boost::system::error_code const& error,
        std::size_t bytes_read
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

    std::deque<std::string> to_write;
};

}


#endif // IRC_CLIENT_HPP
