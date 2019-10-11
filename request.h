#pragma once
#include <string>

namespace forceinline {
	namespace http {
		class request {
		public:
			enum class methods {
				GET = 0,
				POST
			};

			request( const methods request_method, std::string_view request_url )
				: m_method( request_method ), m_request_url( request_url ) { }

			void set_hostname( std::string_view hostname );
			void set_argument( std::string_view arg_name, std::string_view arg_value );

			std::string get( );

		private:
			std::string url_escape( std::string_view to_escape );

			methods m_method = methods::GET;
			std::string m_arguments = { }, m_hostname = { }, m_request_url = { };
		};
	} // namespace http
} // namespace forceinline