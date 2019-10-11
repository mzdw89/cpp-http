#pragma once
#include "request.h"
#include "response.h"

#include <vector>
#include <functional>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

namespace forceinline {
	namespace http {
		class connection {
		public:
			connection( std::string_view hostname ) : m_hostname( hostname ) { }
			~connection( );

			void connect( );
			void close( );

			void make_request( http::request* request, std::function< void( const http::response& ) > callback = nullptr );

		private:
			WSADATA m_wsa_data = { };

			bool m_connected = false;

			std::string m_hostname = { };

			SOCKET m_socket = INVALID_SOCKET;

			std::vector< char > m_buffer = std::vector< char >( 1024 );
		};
	} // namespace http
} // namespace forceinline