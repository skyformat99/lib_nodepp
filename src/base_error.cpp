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
			namespace {
				template<typename T>
				std::unique_ptr<std::decay_t<T>> copy_unique_ptr( std::unique_ptr<T> const &ptr ) {
					if( ptr ) {
						return std::make_unique<std::decay_t<T>>( *ptr );
					}
					return std::unique_ptr<std::decay_t<T>>{};
				}
			} // namespace
			Error::Error( Error const &other )
			    : m_keyvalues{other.m_keyvalues}
			    , m_child{copy_unique_ptr( other.m_child )}
			    , m_exception{other.m_exception}
			    , m_frozen{other.m_frozen} {}

			Error &Error::operator=( Error const &rhs ) {
				if( this == &rhs ) {
					return *this;
				}
				m_keyvalues = rhs.m_keyvalues;
				m_child = copy_unique_ptr( rhs.m_child );
				m_exception = rhs.m_exception;
				m_frozen = rhs.m_frozen;
				return *this;
			}

			Error::Error( daw::string_view description ) : m_child{nullptr}, m_frozen{false} {
				add( "description", description.to_string( ) );
			}

			Error::Error( daw::string_view description, ErrorCode const &err ) : Error{description} {

				add( "message", err.message( ) );
				add( "category", std::string{err.category( ).name( )} );
				add( "error_code", std::to_string( err.value( ) ) );
			}

			Error::Error( daw::string_view description, std::exception_ptr ex_ptr ) : Error{description} {

				m_exception = std::move( ex_ptr );
			}

			Error &Error::add( daw::string_view name, daw::string_view value ) {
				daw::exception::daw_throw_on_true( m_frozen, "Attempt to change a frozen Error." );

				m_keyvalues.push_back( std::pair<std::string, std::string>{name.to_string( ), value.to_string( )} );
				return *this;
			}

			daw::string_view Error::get( daw::string_view name ) const {
				auto pos = std::find_if( m_keyvalues.cbegin( ), m_keyvalues.cend( ),
				                         [name]( auto const &current_value ) { return current_value.first == name; } );
				daw::exception::daw_throw_on_false<std::out_of_range>( pos != m_keyvalues.cend( ),
				                                                       "Name does not exist in Error key values" );
				return pos->second;
			}

			void Error::freeze( ) {
				m_frozen = true;
			}

			Error const &Error::child( ) const {
				return *m_child;
			}

			bool Error::has_child( ) const {
				return static_cast<bool>( m_child );
			}

			bool Error::has_exception( ) const {
				return static_cast<bool>( m_exception ) || ( has_child( ) && child( ).has_exception( ) );
			}

			void Error::throw_exception( ) {
				if( has_child( ) && child( ).has_exception( ) ) {
					m_child->throw_exception( );
				}
				if( has_exception( ) ) {
					auto current_exception = std::exchange( m_exception, nullptr );
					std::rethrow_exception( current_exception );
				}
			}

			void Error::add_child( Error const &child ) {
				daw::exception::daw_throw_on_true( m_frozen, "Attempt to change a frozen Error." );
				freeze( );
				m_child = std::make_unique<Error>( child );
			}

			std::string Error::to_string( daw::string_view prefix ) const {
				auto const no_description = [&]( ) {
					return std::find_if( m_keyvalues.cbegin( ), m_keyvalues.cend( ), []( auto const &current_value ) {
						       return current_value.first == "description";
					       } ) == m_keyvalues.cend( );
				}( );
				if( no_description ) {
					return prefix + "Error: Invalid Error\n";
				}
				std::stringstream ss;
				for( auto const &row : m_keyvalues ) {
					ss << prefix << "'" << row.first << "',	'" << row.second << "'\n";
				}
				if( m_exception ) {
					try {
						std::rethrow_exception( m_exception );
					} catch( std::exception const &ex ) {
						ss << "Exception message: " << ex.what( ) << '\n';
					} catch( Error const &err ) {
						ss << "Exception message: " << err.to_string( ) << '\n';
					} catch( ... ) { ss << "Unknown exception\n"; }
				}
				if( has_child( ) ) {
					auto const p = prefix.to_string( ) + "# ";
					ss << child( ).to_string( p );
				}
				ss << '\n';
				return ss.str( );
			}

			std::ostream &operator<<( std::ostream &os, Error const &error ) {
				os << error.to_string( );
				return os;
			}

			OptionalError create_optional_error( ) {
				return OptionalError{};
			}
		} // namespace base
	}     // namespace nodepp
} // namespace daw
