#include "StdAfx.h"
#include <boost/asio.hpp>
using namespace boost;

using boost::asio::ip::tcp;

void chat_client(tcp::socket& socket, const std::string& message) {
	asio::write(socket, asio::buffer(message + "\n"));
}

void read_messages(tcp::socket& socket) {
	try {
		asio::streambuf buffer;
		while (true) {
			asio::read_until(socket, buffer, "\n");
			std::istream is(&buffer);
			std::string message;
			std::getline(is, message);
			std::cout << "Received: " << message << std::endl;
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

int main() {
	try {
		asio::io_context io_context;

		std::cout << "Enter Server Address: ";
		std::string server; // 서버 IP 주소
		std::getline(std::cin, server);

		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve(server, "12345");

		tcp::socket socket(io_context);
		asio::connect(socket, endpoints);

		std::cout << "Enter your name: ";
		std::string name;
		std::getline(std::cin, name);
		chat_client(socket, name); // 서버에 이름 전송

		// 메시지 수신을 위한 별도의 스레드 실행
		std::thread(read_messages, std::ref(socket)).detach();

		std::string message;
		while (true) {
			std::getline(std::cin, message);
			if (message == "exit") break;

			chat_client(socket, message);
		}

	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
