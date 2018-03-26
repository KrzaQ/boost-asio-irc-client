#include <ctime>
#include <iostream>
#include <regex>
#include <sstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "irc_client.hpp"
#include "extract_regex_groups.hpp"

using namespace std::literals;

namespace asio = boost::asio;

void say_time(
    kq::irc::client& client,
    std::string_view who,
    std::string_view where,
    std::string_view message
) {
    std::string nick;
    kq::extract_regex_groups(
        who.data(),
        std::regex{"([^!:]+)"},
        std::tie(nick)
    );

    std::cout << "NICK: " << nick << std::endl;

    std::string receiver;
    if(where.size() > 1 && where[1] == '#') {
        receiver = std::string{where};
    } else {
        receiver = nick;
    }

    if(message != "!time"){
        return;
    }

    auto result = std::time(nullptr);
    std::stringstream reply;
    reply << nick << ": "
          << std::asctime(std::localtime(&result));
    client.say(receiver, reply.str());
}

void greet(
    kq::irc::client& client,
    std::string_view who,
    std::string_view where,
    std::string_view message
) {
    std::string nick;
    kq::extract_regex_groups(
        who.data(),
        std::regex{"([^!:]+)"},
        std::tie(nick)
    );
    boost::algorithm::trim(nick);

    std::cout << "NICK: " << nick << std::endl;

    if(nick == client.get_settings().nick) {
        return;
    }

    std::string dest{where};
    boost::algorithm::trim(dest);
    client.say(dest, "Hello, " + nick + "!");
}

int main()
{
    asio::io_context io;

    kq::irc::settings settings{
        "irc.freenode.org",
        6667,
        "ProgMag"
    };

    kq::irc::client irc{io, settings};

    irc.register_handler("001", [&](auto&&...){
        irc.join("#progmag");
    });

    irc.register_handler("PRIVMSG", [&](auto&&... views){
        say_time(irc, views...);
    });

    irc.register_handler("JOIN", [&](auto&&... views){
        greet(irc, views...);
    });

    io.run();
}
