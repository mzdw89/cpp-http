# cpp-http 
A lightweight wrapper for HTTP communication. 

## Example usage
```c++
using namespace http = forceinline::http;
http::connection yoursite( "www.yoursite.com" );

try {
	// Connect to our HTTP server
	yoursite.connect( );
	
	// Create a new request
	http::request post_req( http::request::methods::POST, "/some/page.html" );
	post_req.set_argument( "some_argument", "some_value" );

	// Send the request
	yoursite.make_request( &post_req, [ ]( const http::response& response ) {
		// Check the request code
		if ( response.code != 200 )
			return;
		
		// Do something with the body
		std::cout << response.body << std::endl;
	} );

	// Close the connection after we're done
	yoursite.close( );
} catch ( const std::exception& e ) {
	std::cout << e.what( ) << std::endl; 
}
```

## Notes 
This project is not completely finished; it may be performing slowly or undefined behaviour may occur. 

Known issues: 
- Connection will not be closed by client if the "Connection" header is set to "close" and may be closed unwantedly if it is set to "keep-alive". 

## License
This project is licensed under the MIT License. See the license file for further information.
