#include <istream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "irc_client.hpp"
#include "extract_regex_groups.hpp"

namespace kq::irc
{

client::client(
        asio::io_context& ctx,
        irc::settings const& settings
):
    ctx{ctx},
    settings{settings},
    socket{ctx}
{
    register_handler(
        "PING",
        [this](auto, auto, std::string_view ping) {
            std::stringstream pong;
            pong << "PONG :" << ping;
            send_line(pong.str());
        }
    );

    connect();
}

void client::join(std::string_view channel)
{
    std::stringstream msg;
    msg << "JOIN " << channel;
    send_line(msg.str());
}

void client::say(
    std::string_view receiver,
    std::string_view message
) {
    std::stringstream msg;
    msg << "PRIVMSG " << receiver << " :" << message;
    send_line(msg.str());
}

void client::send_line(std::string data)
{
    std::cout << "Sending: " << data << std::endl;

    data += "\r\n";
    to_write.push_back(std::move(data));

    if(to_write.size() == 1)
        send_raw();
}

void client::register_handler(
    std::string name,
    message_handler handler
) {
    handlers[std::move(name)].push_back(handler);
}

void client::register_on_connect(
    std::function<void()> handler
) {
    on_connect_handlers.push_back(handler);
}

void client::connect()
{
    socket.close();
    tcp::resolver resolver(ctx);

    auto handler = [this](auto&&... params) {
        on_hostname_resolved(
            std::forward<decltype(params)>(params)...
        );
    };

    resolver.async_resolve(
        settings.host,
        std::to_string(settings.port),
        handler
    );
}

void client::identify()
{
    std::stringstream msg;
    msg << "USER " << settings.nick << " "
           "foo bar :" << settings.nick;
    send_line(msg.str());

    msg.str("");
    msg << "NICK " << settings.nick;
    send_line(msg.str());
}

void client::on_hostname_resolved(
    boost::system::error_code const& error,
    tcp::resolver::results_type results
) {
    if(error) {
        connect();
        return;
    }

    if(!results.size()) {
        std::stringstream msg;
        msg << "Failed to resolve '" << settings.host << "'";
        throw std::runtime_error(msg.str());
    }

    auto handler = [this](auto const& error) {
        on_connected(error);
    };

    socket.async_connect(*results, handler);
}

void client::on_connected(
    boost::system::error_code const& error
) {
    if(error) {
        connect();
        return;
    }

    std::cout << "Connected.\n";

    identify();

    for(auto& handler : on_connect_handlers) {
        handler();
    }

    await_new_line();
}

void client::await_new_line()
{
    auto handler = [this](auto const& error, std::size_t s) {
        if(error) {
            connect();
            return;
        }

        std::istream i{&in_buf};
        std::string line;
        std::getline(i, line);

        on_new_line(line);
        await_new_line();
    };
    asio::async_read_until(socket, in_buf, "\r\n", handler);
}

void client::on_new_line(
    std::string const& line
) {
    std::cout << "Received: " << line << std::endl;

    // samples of this regex:
    // https://regex101.com/r/rmIXQS/1
    static auto constexpr server_message =
        R"((?::([^@!\ ]*(?:(?:![^@]*)?@[^\ ]*)?)\ ))"
        R"(?([^\ ]+)((?:[^:\ ][^\ ]*)?(?:\ [^:\ ][^\ ]*))"
        R"({0,14})(?:\ :?(.*))?)";

    std::string who, type, where, message;
    kq::extract_regex_groups(
        line.c_str(),
        std::regex{server_message},
        std::tie(who, type, where, message)
    );

    std::cout << ">" << who << "< "
                 ">" << type << "< "
                 ">" << where << "< "
                 ">" << message << "<"
              << std::endl;

    handle_message(who, type, where, message);
}

void client::handle_message(
    std::string_view who,
    std::string const& type,
    std::string_view where,
    std::string_view message
) {
    for(auto const& h : handlers[type]) {
        h(who, where, message);
    }
}

void client::send_raw()
{
    if(to_write.empty()) {
        return;
    }

    socket.async_send(
        asio::buffer(
            to_write.front().data(),
            to_write.front().size()
        ),
        [this](auto&&... params){
            handle_write(params...);
        }
    );
}

void client::handle_write(
    boost::system::error_code const& error,
    std::size_t bytes_read
) {
    if(error) {
        std::cerr << "Error: " << error << std::endl;
        return;
    }

    auto to_erase =
        std::min(bytes_read, to_write.front().size());

    auto& buf = to_write.front();

    buf.erase(buf.begin(), buf.begin() + to_erase);

    if(buf.empty()) {
        to_write.erase(to_write.begin());
    }

    if(!to_write.empty()) {
        send_raw();
    }
}

} // kq::irc
