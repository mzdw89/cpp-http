#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace forceinline {
	namespace http {
		struct headers {
			std::string operator[]( std::string_view header_name ) {
				return std::find_if( container.begin( ), container.end( ), [ & ]( const std::pair< std::string, std::string >& p ) {
					return p.first == header_name;
				} )->second;
			}

			bool exists( std::string_view header_name ) {
				return std::find_if( container.begin( ), container.end( ), [ & ]( const std::pair< std::string, std::string >& p ) {
					return p.first == header_name;
				} ) != container.end( );
			}

			std::vector< std::pair< std::string, std::string > > container = { };
		};

		struct response {
			// Parses the response
			response( const std::string& response, bool* transmission_finished ) {
				auto http_headers_end = response.find( "\r\n\r\n" );
				auto http_headers = response.substr( 0, http_headers_end ) + "\r\n";

				auto split_string = [ ]( std::string to_split, std::string_view by ) {
					std::vector< std::string > result = { };

					for ( auto next = to_split.find( by ); next != std::string::npos; next = to_split.find( by ) ) {
						result.push_back( to_split.substr( 0, next ) );
						to_split = to_split.substr( next + by.length( ) );
					}

					return result;
				};

				auto split_headers = split_string( http_headers, "\r\n" );

				// Parse HTTP information header manually		
				{
					auto& http_information_header = split_headers[ 0 ];
					auto http_version_split = http_information_header.find( ' ' );

					auto http_version = http_information_header.substr( 5, http_version_split - 5 );
					code = std::stoi( http_information_header.substr( http_version_split + 1, 3 ) );

					headers.container.push_back( { "HTTP", http_version } );
				}

				// Store the headers
				for ( auto http_header = split_headers.begin( ) + 1; http_header != split_headers.end( ); http_header++ ) {
					if ( http_header->empty( ) )
						continue;

					auto header_splitter = http_header->find( ": " );
					auto header_name = http_header->substr( 0, header_splitter );
					auto header_value = http_header->substr( header_splitter + 2 );

					header_value.erase( std::remove( header_value.begin( ), header_value.end( ), '\"' ) );

					headers.container.push_back( { header_name, header_value } );
				}

				// Copy the body
				body = response.substr( http_headers_end + 4 );

				// The content lenght was given, check if the transmission finished by checking the body length
				if ( headers.exists( "Content-Length" ) )
					*transmission_finished = body.length( ) >= std::size_t( std::stoi( headers[ "Content-Length" ] ) );
				else {
					// Chunked transmission
					
					// We need to have the transfer encoding marked as chunked if we didn't receive a content length
					if ( !headers.exists( "Transfer-Encoding" ) || headers[ "Transfer-Encoding" ] != "chunked" )
						throw std::exception( "response::response: end of transfer unknown" );

					// End of transmission in chunked encoding is marked via the following chunk at the end
					std::string chunk_end = "0\r\n\r\n";
					if ( body.find( "0\r\n\r\n" ) == body.length( ) - chunk_end.length( ) )
						*transmission_finished = true;
				}
			}

			std::string body = { };
			std::uint16_t code = 0;
			http::headers headers = { };
		};
	} // namespace http
} // namespace forceinline