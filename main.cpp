#include <ctime>
#include <iostream>
#include <regex>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "irc_client.hpp"
#include "regex_explode.hpp"

using namespace std::literals;

namespace asio = boost::asio;

void say_time(
    kq::irc::client& client,
    std::string_view who,
    std::string_view where,
    std::string_view message
) {
    std::string nick;
    kq::explode_regex(
        who.data(),
        "([^!:]+)",
        std::tie(nick)
    );

    std::cout << "NICK: " << nick << std::endl;

    std::string receiver;
    if(where.size() > 1 && where[1] == '#') {
        receiver = std::string{where};
    } else {
        receiver = nick;
    }

    if(message != ":time"){
        return;
    }

    std::time_t result = std::time(nullptr);
    std::stringstream reply;
    reply << nick << ": "
          << std::asctime(std::localtime(&result));
    client.say(receiver, reply.str());
}

int main()
{
    asio::io_service io;

    kq::irc::settings settings{
        "irc.quakenet.org",
        6667,
        "Vesumon"
    };

    kq::irc::client irc{io, settings};

    irc.register_handler("001", [&](auto&&...){
        irc.join("#krzaq");
    });

    irc.register_handler("PRIVMSG", [&](auto&&... views){
        say_time(irc, views...);
    });

    io.run();
}
