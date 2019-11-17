#include "connection.h"
#include <algorithm>

namespace forceinline {
	http::connection::~connection( ) {
		if ( !m_connected )
			return;

		close( );
	}

	void http::connection::connect( ) {
		if ( WSAStartup( MAKEWORD( 2, 2 ), &m_wsa_data ) != 0 )
			throw std::exception( "http::connection::connect: WSAStartup failed" );

		addrinfo* result = nullptr, hints = { };

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the hostname
		if ( getaddrinfo( m_hostname.data( ), "80", &hints, &result ) != 0 )
			throw std::exception( "http::connection::connect: failed to resolve hostname" );

		// Create a socket
		m_socket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );

		if ( m_socket == INVALID_SOCKET )
			throw std::exception( "http::connection::connect: failed to create socket" );

		// Connect to the HTTP server
		if ( ::connect( m_socket, result->ai_addr, result->ai_addrlen ) != 0 )
			throw std::exception( "http::connection::connect: failed to connect to host" );

		// Remember to free up the memory at addrinfo
		freeaddrinfo( result );

		// Mark us as connected
		m_connected = true;
	}

	void http::connection::close( ) {
		// Shutdown the connection properly
		shutdown( m_socket, SD_BOTH );
		m_socket = INVALID_SOCKET;

		WSACleanup( );
		m_connected = false;
	}

	void http::connection::make_request( http::request* request, std::function< void( const http::response& ) > callback ) {
		if ( !request )
			throw std::exception( "http::connection::make_request: request is nullptr" );

		// Set the hostname because it's required in HTTP version 1.1
		request->set_hostname( m_hostname );

		// Get the finalized request
		auto final_request = request->get( );

		// Attempt to send the request
		std::size_t total_bytes_sent = 0;
		do {
			int bytes_sent = send( m_socket, final_request.data( ) + total_bytes_sent, final_request.length( ) - total_bytes_sent, 0 );

			// Some error occurred, notify the user
			if ( bytes_sent <= 0 )
				throw std::exception( "http::connection::make_request: error sending request" );
			else
				total_bytes_sent += bytes_sent;
		} while ( total_bytes_sent < final_request.length( ) );

		// Attempt to receive our response
		int bytes_received = 0;
		std::string response_body = { };
		bool transmission_finished = false;
		do {
			bytes_received = recv( m_socket, m_buffer.data( ), m_buffer.capacity( ), 0 );

			if ( bytes_received <= 0 )
				throw std::exception( "connection::make_request: error receiving response" );

			response_body.insert( response_body.end( ), m_buffer.begin( ), m_buffer.begin( ) + bytes_received );
			if ( auto header_end = response_body.find( "\r\n\r\n" ); header_end != std::string::npos ) {
				http::response response( response_body, &transmission_finished );

				if ( transmission_finished && callback ) {
					callback( response );
					break;
				}
			}
		} while ( !transmission_finished );
	}
} // namespace forceinline