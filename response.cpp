#include "response.h"

namespace forceinline {
	namespace http {
		std::string headers::operator[]( std::string_view header_name ) const {
			return std::find_if( container.begin( ), container.end( ), [ & ]( const std::pair< std::string, std::string >& p ) {
				return p.first == header_name;
			} )->second;
		}

		bool headers::exists( std::string_view header_name ) const {
			return std::find_if( container.begin( ), container.end( ), [ & ]( const std::pair< std::string, std::string >& p ) {
				return p.first == header_name;
			} ) != container.end( );
		}

		// Parses the response
		response::response( const std::string& response, bool* transmission_finished ) {
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
				* transmission_finished = body.length( ) >= std::size_t( std::stoi( headers[ "Content-Length" ] ) );
			else {
				// Chunked transmission; we need to have the transfer encoding marked as chunked if we didn't receive a content length
				if ( !headers.exists( "Transfer-Encoding" ) || headers[ "Transfer-Encoding" ] != "chunked" )
					throw std::exception( "http::response::response: end of transfer unknown" );

				auto chunks = split_string( body, "\r\n" );
				body.clear( );

				// We have to receive a even number of chunks
				if ( chunks.size( ) % 2 != 0 )
					return;

				// Iterate chunks; skip body.
				bool received_last_chunk = false;
				for ( auto chunk = chunks.begin( ); chunk != chunks.end( ); chunk += 2 ) {
					auto chunk_length = std::stoull( chunk->data( ), nullptr, 16 );
					auto chunk_body = chunk + 1;

					// We received the last chunk.
					if ( chunk_length == 0 && chunk_body->length( ) == 0 ) {
						received_last_chunk = true;
						break;
					}

					// Chunk length must match body length
					if ( chunk_body->length( ) != chunk_length )
						return;

					// Append to body
					body.append( chunk_body->data( ) );
				}

				*transmission_finished = received_last_chunk;
			}
		}
	} // namespace http
} // namespace forceinline