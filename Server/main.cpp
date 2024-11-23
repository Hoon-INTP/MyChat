#include "StdAfx.h"
#include <boost/asio.hpp>
using namespace boost;

using boost::asio::ip::tcp;

class ChatSession; // Forward declaration

std::vector<std::shared_ptr<ChatSession>> clients; // 클라이언트 세션 목록
std::mutex clients_mutex; // 클라이언트 목록 보호를 위한 뮤텍스

class ChatSession : public std::enable_shared_from_this<ChatSession> {
public:
	ChatSession(tcp::socket socket)
		: socket_(std::make_shared<tcp::socket>(std::move(socket))) {
	}

	void start() {
		{
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.push_back(shared_from_this());
		}
		std::cout << "Client connected: " << socket_->remote_endpoint().address().to_string()
			<< ":" << socket_->remote_endpoint().port() << std::endl;
		do_read_name();
	}

	void deliver(const std::string& msg) {
		asio::async_write(*socket_, asio::buffer(msg),
			[self = shared_from_this()](std::error_code ec, std::size_t /*length*/) {
				if (ec) {
					self->handle_disconnect();
				}
			});
	}

private:
	void do_read_name() {
		auto self(shared_from_this());
		asio::async_read_until(*socket_, asio::dynamic_buffer(data_), "\n",
			[this, self](std::error_code ec, std::size_t length) {
				if (!ec) {
					name_ = data_.substr(0, length - 1); // 클라이언트 이름 설정
					data_.erase(0, length);
					do_read();
				}
				else {
					handle_disconnect();
				}
			});
	}

	void do_read() {
		auto self(shared_from_this());
		asio::async_read_until(*socket_, asio::dynamic_buffer(data_), "\n",
			[this, self](std::error_code ec, std::size_t length) {
				if (!ec) {
					std::string message = name_ + ": " + data_.substr(0, length);
					data_.erase(0, length);
					{
						std::lock_guard<std::mutex> lock(clients_mutex);
						for (auto& client : clients) {
							if (client != self) {
								client->deliver(message);
							}
						}
					}
					do_read();
				}
				else {
					handle_disconnect();
				}
			});
	}

	void handle_disconnect() {
		std::lock_guard<std::mutex> lock(clients_mutex);
		clients.erase(std::remove(clients.begin(), clients.end(), shared_from_this()), clients.end());
		std::cout << "Client disconnected: " << socket_->remote_endpoint().address().to_string()
			<< ":" << socket_->remote_endpoint().port() << std::endl;
	}

	std::shared_ptr<tcp::socket> socket_;
	std::string data_;
	std::string name_;
};

class ChatServer {
public:
	ChatServer(asio::io_context& io_context, short port)
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
		start_accept();
	}

	void start_accept() {
		acceptor_.async_accept(
			[this](std::error_code ec, tcp::socket socket) {
				if (!ec) {
					std::make_shared<ChatSession>(std::move(socket))->start();
				}
				start_accept();
			});
	}

private:
	tcp::acceptor acceptor_;
};

int main() {
	try {
		asio::io_context io_context;
		ChatServer server(io_context, 12345);
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}
