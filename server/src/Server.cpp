#include "Server.hpp"

Server::Server(io_context &io_service, const ip::tcp::endpoint &endpoint,
               ProcessDataCallback procDatacallback)
    : ioService_(io_service), acceptor_(io_service, endpoint),
      procDatacallback_(procDatacallback) {
  startAccept();
}

void Server::startAccept() {
  auto session = std::make_shared<Session>(ioService_, procDatacallback_);

  acceptor_.async_accept(session->socket(),
                         boost::bind(&Server::handleAccept, this, session,
                                     boost::asio::placeholders::error));
}

void Server::handleAccept(std::shared_ptr<Session> session,
                          const boost::system::error_code &error) {
  if (!error) {
    session->start();
    sessions.push_back(session);
  }

  startAccept();
}