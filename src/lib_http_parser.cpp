// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "lib_http_parser.h"
#include "lib_http_parser_impl.h"
#include "lib_http_url.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace http {
				std::shared_ptr<daw::nodepp::lib::http::impl::HttpClientRequestImpl>
				parse_http_request( daw::string_view str ) {
					try {
						return std::make_shared<impl::HttpClientRequestImpl>( parse::http_request_parser( str ) );
					} catch( daw::parser::ParserException const & ) { return nullptr; }
				}

				std::shared_ptr<daw::nodepp::lib::http::HttpAbsoluteUrlPath> parse_url_path( daw::string_view path ) {
					try {
						return std::make_shared<daw::nodepp::lib::http::HttpAbsoluteUrlPath>(
						    daw::nodepp::lib::http::parse::http_absolute_url_path_parser( path ) );
					} catch( daw::parser::ParserException const & ) { return nullptr; }
				}

				std::shared_ptr<daw::nodepp::lib::http::impl::HttpUrlImpl> parse_url( daw::string_view url_string ) {
					try {
						return std::make_shared<daw::nodepp::lib::http::impl::HttpUrlImpl>(
						    daw::nodepp::lib::http::parse::http_url_parser( url_string ) );
					} catch( daw::parser::ParserException const & ) { return nullptr; }
				}

				static constexpr char make_hex( char c ) {
					switch( c ) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						return ( c - '0' );
					case 'a':
					case 'b':
					case 'c':
					case 'd':
					case 'e':
					case 'f':
						return ( c - 'a' ) + 10;
					case 'A':
					case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
						return ( c - 'A' ) + 10;
					}
					daw::exception::daw_throw( "Invalid hex digit" );
				}

				static constexpr char make_hex( daw::string_view str ) {
					auto c = static_cast<char>( static_cast<unsigned char>( make_hex( str.front( ) ) ) << 4u );
					str.remove_prefix( );
					c |= make_hex( str.front( ) );
					return c;
				}

				std::string url_decode( daw::string_view str ) {
					std::string result;
					auto pos = str.find_first_of( '%' );
					while( pos != str.npos ) {
						result += str.substr( 0, pos );
						str.remove_prefix( pos + 1 );

						result += make_hex( str );
						str.remove_prefix( 2 );
						pos = str.find_first_of( '%' );
					}
					result.insert( result.cend( ), str.cbegin( ), str.cend( ) );
					return result;
				}
			} // namespace http
		}     // namespace lib
	}         // namespace nodepp
	;
} // namespace daw
