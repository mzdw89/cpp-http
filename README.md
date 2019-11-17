# cpp-http 
A lightweight wrapper for HTTP communication. 

## Notes 
This project is not completely finished; it may be performing slowly or undefined behaviour may occur. 

Known issues: 
- Connection will not be closed by client if the "Connection" header is set to "close" and may be closed unwantedly if it is set to "keep-alive". 

## License This project is licensed under the MIT License. See the license file for further information.
