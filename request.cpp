#include "request.h"
#include <sstream>
#include <atlutil.h>

namespace forceinline {
	void http::request::set_hostname( std::string_view hostname ) {
		m_hostname = hostname;
	}

	void http::request::set_argument( std::string_view arg_name, std::string_view arg_value ) {
		if ( m_method == methods::GET )
			m_arguments += m_arguments.length( ) > 0 ? '&' : '?';
		else if ( m_arguments.length( ) )
			m_arguments += '&';

		m_arguments += url_escape( arg_name ) + '=' + url_escape( arg_value );
	}

	std::string http::request::get( ) {
		std::string request = m_method == methods::GET ? "GET " : "POST ";
		request += m_request_url;

		if ( m_method == methods::GET )
			request += m_arguments;

		request += " HTTP/1.1\r\n";
		request += "Host: " + m_hostname + "\r\n";
		request += m_method == methods::GET ? "Accept: text/*\r\n\r\n" : "Content-Type: application/x-www-form-urlencoded\r\n";

		if ( m_method == methods::POST ) {
			request += "Content-Length: " + std::to_string( m_arguments.length( ) ) + "\r\n\r\n";
			request += m_arguments;
		}

		return request;
	}

	std::string http::request::url_escape( std::string_view to_escape ) {
		auto http_escape_char = [ ]( char c ) {
			std::stringstream ss;
			ss << '%' << std::hex << int( c );
			return ss.str( );
		};

		std::string escaped = { };
		for ( auto& c : to_escape ) {
			if ( !AtlIsUnsafeUrlChar( c ) )
				escaped += c;
			else
				escaped += http_escape_char( c );
		}

		return escaped;
	}
} // namespace forceinline