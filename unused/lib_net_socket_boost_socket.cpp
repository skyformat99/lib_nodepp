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

#include <daw/daw_exception.h>
#include <daw/daw_utility.h>

#include "base_service_handle.h"
#include "lib_net_socket_boost_socket.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace net {
				namespace impl {
					BoostSocket::~BoostSocket( ) = default;

					BoostSocket::BoostSocket( )
					    : m_context{nullptr}
					    , m_socket{std::make_shared<BoostSocketValueType>( base::ServiceHandle::get( ), *m_context )} {}

					BoostSocket::BoostSocket( std::shared_ptr<EncryptionContext> context )
					    : m_context{std::move( context )}
					    , m_socket{std::make_shared<BoostSocketValueType>( base::ServiceHandle::get( ), *m_context )} {}

					BoostSocket::BoostSocket( std::shared_ptr<EncryptionContext> context,
					                          std::shared_ptr<BoostSocketValueType> socket )
					    : m_context{std::move( context )}, m_socket{std::move( socket )} {}

					void BoostSocket::reset_socket( ) {
						m_socket.reset( );
					}

					EncryptionContext &BoostSocket::encryption_context( ) {
						daw::exception::daw_throw_on_false( m_context,
						                                    "Attempt to retrieve an invalid encryption context" );
						return *m_context;
					}

					EncryptionContext const &BoostSocket::encryption_context( ) const {
						daw::exception::daw_throw_on_false( m_context,
						                                    "Attempt to retrieve an invalid encryption context" );
						return *m_context;
					}

					BoostSocket::BoostSocketValueType &BoostSocket::raw_socket( ) {
						return *m_socket;
					}

					BoostSocket::BoostSocketValueType const &BoostSocket::raw_socket( ) const {
						daw::exception::daw_throw_on_false( m_socket, "Invalid socket" );
						return *m_socket;
					}

					BoostSocket::operator bool( ) const {
						return static_cast<bool>( m_socket );
					}

					BoostSocket::BoostSocketValueType const &BoostSocket::operator*( ) const {
						return raw_socket( );
					}

					BoostSocket::BoostSocketValueType &BoostSocket::operator*( ) {
						return raw_socket( );
					}

					BoostSocket::BoostSocketValueType *BoostSocket::operator->( ) const {
						daw::exception::daw_throw_on_false( m_socket, "Invalid socket - null" );
						return m_socket.operator->( );
					}

					BoostSocket::BoostSocketValueType *BoostSocket::operator->( ) {
						return m_socket.operator->( );
					}

					bool BoostSocket::encryption_on( ) const {
						return static_cast<bool>( m_context );
					}

					std::shared_ptr<EncryptionContext> BoostSocket::context( ) {
						return m_context;
					}

					std::shared_ptr<EncryptionContext> const &BoostSocket::context( ) const {
						return m_context;
					}

					bool BoostSocket::is_open( ) const {
						return raw_socket( ).next_layer( ).is_open( );
					}

					void BoostSocket::shutdown( ) {
						if( this->encryption_on( ) ) {
							raw_socket( ).shutdown( );
						}
						raw_socket( ).lowest_layer( ).shutdown( boost::asio::socket_base::shutdown_both );
					}

					boost::system::error_code BoostSocket::shutdown( boost::system::error_code &ec ) noexcept {
						if( this->encryption_on( ) ) {
							raw_socket( ).shutdown( );
							ec = raw_socket( ).shutdown( ec );
							if( static_cast<bool>( ec ) ) {
								return ec;
							}
						}
						return raw_socket( ).lowest_layer( ).shutdown( boost::asio::socket_base::shutdown_both, ec );
					}

					void BoostSocket::close( ) {
						raw_socket( ).shutdown( );
					}

					boost::system::error_code BoostSocket::close( boost::system::error_code &ec ) {
						return raw_socket( ).shutdown( ec );
					}

					void BoostSocket::cancel( ) {
						raw_socket( ).next_layer( ).cancel( );
					}

					boost::asio::ip::tcp::endpoint BoostSocket::remote_endpoint( ) const {
						return raw_socket( ).next_layer( ).remote_endpoint( );
					}

					boost::asio::ip::tcp::endpoint BoostSocket::local_endpoint( ) const {
						return raw_socket( ).next_layer( ).local_endpoint( );
					}
				} // namespace impl
			}     // namespace net
		}         // namespace lib
	}             // namespace nodepp
} // namespace daw
