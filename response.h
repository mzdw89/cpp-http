#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace forceinline {
	namespace http {
		struct headers {
			std::string operator[]( std::string_view header_name ) const;
			bool exists( std::string_view header_name ) const;

			std::vector< std::pair< std::string, std::string > > container = { };
		};

		struct response {
			response( const std::string& response, bool* transmission_finished );

			std::string body = { };
			std::uint16_t code = 0;
			http::headers headers = { };
		};
	} // namespace http
} // namespace forceinline