// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
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

#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <stdexcept>
#include <string>

#include <daw/daw_string_view.h>

#include "base_error.h"

namespace daw {
	namespace nodepp {
		namespace base {
			Error::~Error( ) = default;

			Error::Error( daw::string_view description ) : m_frozen{false} {

				m_keyvalues.emplace( "description", description.to_string( ) );
			}

			Error::Error( ErrorCode const &err ) : Error{err.message( )} {

				m_keyvalues.emplace( "category", std::string{err.category( ).name( )} );
				m_keyvalues.emplace( "error_code", std::to_string( err.value( ) ) );
			}

			Error::Error( daw::string_view description, std::exception_ptr ex_ptr )
			    : m_frozen{false}, m_exception{std::move( ex_ptr )} {

				m_keyvalues.emplace( "description", description.to_string( ) );
			}

			Error &Error::add( daw::string_view name, daw::string_view value ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				if( m_frozen ) {
					throw std::runtime_error( "Attempt to change a frozen error." );
				}
				m_keyvalues[name.to_string( )] = value.to_string( );
				return *this;
			}

			daw::string_view Error::get( daw::string_view name ) const {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				return m_keyvalues.at( name.to_string( ) );
			}

			std::string &Error::get( daw::string_view name ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				return m_keyvalues[name.to_string( )];
			}

			void Error::freeze( ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				m_frozen = true;
			}

			Error const &Error::child( ) const {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				return m_child.get( );
			}

			Error &Error::child( ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				return m_child.get( );
			}

			bool Error::has_child( ) const {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				return static_cast<bool>( m_child );
			}

			bool Error::has_exception( ) const {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				if( has_child( ) && child( ).has_exception( ) ) {
					return true;
				}
				return static_cast<bool>( m_exception );
			}

			void Error::throw_exception( ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				if( has_child( ) && child( ).has_exception( ) ) {
					child( ).throw_exception( );
				}
				if( has_exception( ) ) {
					auto current_exception = std::move( m_exception );
					m_exception = nullptr;
					std::rethrow_exception( current_exception );
				}
			}

			Error &Error::clear_child( ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				m_child.reset( );
				return *this;
			}

			Error &Error::child( Error child ) {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				child.freeze( );
				m_child = child;
				return *this;
			}

			std::string Error::to_string( daw::string_view prefix ) const {
				daw::exception::daw_throw_on_false( m_keyvalues.find( "description" ) != m_keyvalues.end( ),
				                                    "Could not find description field" );
				std::stringstream ss;
				ss << prefix << "Description: " << m_keyvalues.at( "description" ) << "\n";
				for( auto const &row : m_keyvalues ) {
					if( row.first != "description" ) {
						ss << prefix << "'" << row.first << "',	'" << row.second << "'\n";
					}
				}
				if( m_exception ) {
					try {
						std::rethrow_exception( m_exception );
					} catch( std::exception const &ex ) {
						ss << "Exception message: " << ex.what( ) << "\n";
					} catch( ... ) { ss << "Unknown exception\n"; }
				}
				if( has_child( ) ) {
					ss << child( ).to_string( prefix.to_string( ) + "# " );
				}
				ss << "\n";
				return ss.str( );
			}

			std::ostream &operator<<( std::ostream &os, Error const &error ) {
				os << error.to_string( );
				return os;
			}

			OptionalError create_optional_error( ) {
				return OptionalError( );
			}
		} // namespace base
	}     // namespace nodepp
} // namespace daw
