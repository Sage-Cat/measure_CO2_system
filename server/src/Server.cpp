#include "Server.hpp"

Server::Server(io_context& ioContext, const ip::tcp::endpoint& endpoint, Callback appCallback) 
    : acceptor(ioContext, endpoint), callback(appCallback) {
    startAccept();
}

void Server::startAccept() {
    auto session = std::make_shared<Session>(acceptor.get_executor().context(), callback);
    acceptor.async_accept(session->socket(),
        [this, session](const boost::system::error_code& error) {
            handleAccept(session, error);
        });
}

void Server::handleAccept(std::shared_ptr<Session> session, const boost::system::error_code& error) {
    if (!error) {
        session->start();
        sessions.push_back(session);
    }

    startAccept();
}


Server::Server(io_context& ioContext, const ip::tcp::endpoint& endpoint, Callback appCallback) 
    : acceptor(ioContext, endpoint), callback(appCallback) {
    startAccept();
}
