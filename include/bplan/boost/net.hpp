#pragma once

#include <util/wignore-push.inl>
#  include <boost/asio/ip/tcp.hpp>
#  include <boost/asio/strand.hpp>
#  include <boost/asio/connect.hpp>

#  include <boost/beast/core.hpp>
#  include <boost/beast/http.hpp>
#  include <boost/beast/version.hpp>
#include <util/wignore-pop.inl>

#include <memory>
#include <chrono>
#include <format>
#include <string>
#include <string_view>
#include <functional>

namespace bplan::networking{};

namespace net {
using namespace bplan::networking;
using namespace boost::asio ;
using namespace boost::beast;
      namespace http = boost::beast::http;
using                  boost::asio::ip::tcp;
//using                boost::asio::ip::udp;

// Known namespace collsisions: string_view
} // namespace net


namespace bplan::networking {

constexpr auto PORT_HTTP  =  "80";
constexpr auto PORT_HTTPS = "443";

template<class Response> class Session : public std::enable_shared_from_this<Session<Response>>
{
	using shared = std::enable_shared_from_this<Session<Response>>;

	net::http::request<net::http::empty_body> request{};
	/*       */Response                      &response ;
	net::tcp::resolver resolver;
	net::tcp_stream tstream;
	net::flat_buffer buffer{}; // (Must persist between reads)

	std::chrono::steady_clock::duration timeout{};
	std::string *error;

private:
	void fail(net::error_code ec, char const *what) { if (this->error) *this->error = std::format("{}: {}", what, ec.message()); } // Report a failure

public:
	Session           (const Session&) = delete;
	Session& operator=(const Session&) = delete;

	// Objects are constructed with a strand to ensure that handlers do not execute concurrently
	Session(net::io_context &ioc, Response &response, std::string *error = nullptr) : response{response}, resolver{net::make_strand(ioc)}, tstream{net::make_strand(ioc)}, error{error} {}

	// Queue the asynchronous operation // queue async I/O operation
	void Set(std::string_view host, std::string_view port, std::string_view target, std::chrono::steady_clock::duration timeout = std::chrono::seconds{0}) {
		this->timeout = timeout;
		this->tstream.expires_never();
		if (this->error) this->error->clear();

		// Set up an HTTP GET request message
		request.method(net::http::verb::get);
		request.version(11);
		request.set(net::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		request.set(net::http::field::host, host);
		request.target(target);
		request.set(net::http::field::accept, "*/*");

		//std::cout << "\n" << request.base() << "\n";

		// Look up the domain name
		resolver.async_resolve(host, port, std::bind_front(&Session::OnResolve, shared::shared_from_this())); // queue async I/O operation
	}

	void OnResolve(net::error_code ec, net::tcp::resolver::results_type resolves) {
		using namespace std::literals::chrono_literals;
		if (ec) return fail(ec, "resolve");
		if (timeout != 0s) tstream.expires_after(timeout); // Set a timeout on the operation
		tstream.async_connect(resolves, std::bind_front(&Session::OnConnect, shared::shared_from_this())); // Make the connection on the IP address we get from a lookup
	}

	void OnConnect(net::error_code ec, net::tcp::resolver::results_type::endpoint_type) {
		if (ec) return fail(ec, "connect");
		net::http::async_write(tstream, request, std::bind_front(&Session::OnWrite, shared::shared_from_this())); // Send the HTTP request to the remote host
	}

	void OnWrite  (net::error_code ec, size_t) {
		if (ec) return fail(ec, "write");
		net::http::async_read(tstream, buffer, response, std::bind_front(&Session::OnRead, shared::shared_from_this())); // Receive the HTTP response
	}

	void OnRead   (net::error_code ec, size_t) {
		if (ec) return fail(ec, "read");
		tstream.socket().shutdown(net::tcp::socket::shutdown_both, ec); // Gracefully close the socket

		if (ec && ec != net::errc::not_connected) return fail(ec, "shutdown");
	}
};


} // namespace bplan::networking
