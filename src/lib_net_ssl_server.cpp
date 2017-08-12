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

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include <daw/daw_exception.h>
#include <daw/daw_range_algorithm.h>

#include "base_event_emitter.h"
#include "base_service_handle.h"
#include "base_types.h"
#include "lib_net_server.h"
#include "lib_net_socket_stream.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace net {
				namespace impl {
					using NetSslServer = std::shared_ptr<NetSslServerImpl>;
					using namespace daw::nodepp;
					using namespace boost::asio::ip;

					NetSslServerImpl::NetSslServerImpl( daw::nodepp::lib::net::SSLConfig const &ssl_config,
					                                    daw::nodepp::base::EventEmitter emitter )
					    : daw::nodepp::base::StandardEvents<NetSslServerImpl>{std::move( emitter )}
					    , m_acceptor{std::make_shared<boost::asio::ip::tcp::acceptor>( base::ServiceHandle::get( ) )}
					    , m_context{std::make_shared<EncryptionContext>( EncryptionContext::tlsv12_server )} {

						m_context->set_options( EncryptionContext::default_workarounds | EncryptionContext::no_sslv2 |
						                        EncryptionContext::no_sslv3 | EncryptionContext::single_dh_use );

						if( !ssl_config.tls_certificate_chain_file.empty( ) ) {
							m_context->use_certificate_chain_file( ssl_config.get_tls_certificate_chain_file( ) );
						}
						if( !ssl_config.tls_private_key_file.empty( ) ) {
							m_context->use_private_key_file( ssl_config.get_tls_private_key_file( ),
							                                 EncryptionContext::file_format::pem );
						}
						if( !ssl_config.tls_dh_file.empty( ) ) {
							m_context->use_tmp_dh_file( ssl_config.get_tls_dh_file( ) );
						}
					}

					NetSslServerImpl::~NetSslServerImpl( ) = default;

					EncryptionContext &NetSslServerImpl::ssl_context( ) {
						return *m_context;
					}

					EncryptionContext const &NetSslServerImpl::ssl_context( ) const {
						return *m_context;
					}

					bool NetSslServerImpl::using_ssl( ) const {
						return static_cast<bool>( m_context );
					}

					void NetSslServerImpl::listen( uint16_t port ) {
						emit_error_on_throw(
						    get_ptr( ), "Error listening for connection", "NetSslServerImpl::listen", [&]( ) {
							    auto endpoint = EndPoint( boost::asio::ip::tcp::v4( ), port );
							    m_acceptor->open( endpoint.protocol( ) );
							    m_acceptor->set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
							    m_acceptor->bind( endpoint );
							    m_acceptor->listen( 511 );
							    start_accept( );
							    emit_listening( std::move( endpoint ) );
						    } );
					}

					void NetSslServerImpl::close( ) {
						daw::exception::daw_throw_not_implemented( );
					}

					daw::nodepp::lib::net::NetAddress const &NetSslServerImpl::address( ) const {
						daw::exception::daw_throw_not_implemented( );
					}

					void NetSslServerImpl::set_max_connections( uint16_t value ) {
						Unused( value );
						daw::exception::daw_throw_not_implemented( );
					}

					void NetSslServerImpl::get_connections(
					    std::function<void( base::Error err, uint16_t count )> callback ) {
						Unused( callback );
						daw::exception::daw_throw_not_implemented( );
					}

					// Event callbacks
					NetSslServerImpl &
					NetSslServerImpl::on_connection( std::function<void( NetSocketStream socket )> listener ) {
						emitter( )->add_listener( "connection", std::move( listener ) );
						return *this;
					}

					NetSslServerImpl &
					NetSslServerImpl::on_next_connection( std::function<void( NetSocketStream socket_ptr )> listener ) {
						emitter( )->add_listener( "connection", std::move( listener ), true );
						return *this;
					}

					NetSslServerImpl &NetSslServerImpl::on_listening( std::function<void( EndPoint )> listener ) {
						emitter( )->add_listener( "listening", std::move( listener ) );
						return *this;
					}

					NetSslServerImpl &NetSslServerImpl::on_next_listening( std::function<void( )> listener ) {
						emitter( )->add_listener( "listening", std::move( listener ), true );
						return *this;
					}

					NetSslServerImpl &NetSslServerImpl::on_closed( std::function<void( )> listener ) {
						emitter( )->add_listener( "closed", std::move( listener ), true );
						return *this;
					}

					void NetSslServerImpl::handle_handshake( std::weak_ptr<NetSslServerImpl> obj,
					                                         NetSocketStream socket, base::ErrorCode const &err ) {}

					void NetSslServerImpl::handle_accept( std::weak_ptr<NetSslServerImpl> obj, NetSocketStream socket,
					                                      base::ErrorCode const &err ) {
						run_if_valid(
						    std::move( obj ), "Exception while accepting connections",
						    "NetSslServerImpl::handle_accept",
						    [ socket = std::move( socket ), &err ]( NetSslServer self ) mutable {
							    if( !err ) {
								    try {
									    socket->socket( ).async_handshake( boost::asio::ssl::stream_base::server, [
										    obj = self->get_weak_ptr( ), socket = std::move( socket )
									    ]( base::ErrorCode const &err1 ) mutable {
										    handle_handshake( obj, socket, err1 );
									    } );
								    } catch( ... ) {
									    self->emit_error( std::current_exception( ), "Running connection listeners",
									                      "NetSslServerImpl::listen#emit_connection" );
								    }
							    } else {
								    self->emit_error( err, "Running connection listeners", "NetSslServerImpl::listen" );
							    }
							    self->start_accept( );
						    } );
					}

					namespace {
						template<typename Handler>
						void async_accept( std::shared_ptr<boost::asio::ip::tcp::acceptor> &acceptor,
						                   boost::asio::ip::tcp::socket &socket, Handler handler ) {
							acceptor->async_accept( socket, handler );
						}

						template<typename Handler>
						void async_accept( std::shared_ptr<boost::asio::ip::tcp::acceptor> &acceptor,
						                   boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &socket,
						                   Handler handler ) {
							acceptor->async_accept( socket.next_layer( ), handler );
						}
					} // namespace

					void NetSslServerImpl::start_accept( ) {
						emit_error_on_throw(
						    get_ptr( ), "Error while starting accept", "NetSslServerImpl::start_accept", [&]( ) {
							    NetSocketStream socket_sp{nullptr};
							    socket_sp = daw::nodepp::lib::net::create_net_socket_stream( m_context );

							    daw::exception::daw_throw_on_false(
							        socket_sp, "NetSslServerImpl::start_accept( ), Invalid socket - null" );

							    socket_sp->socket( ).init( );
							    auto &boost_socket = socket_sp->socket( );
							    auto async_accept_handler =
							        [ obj = this->get_weak_ptr( ),
								      socket_sp = std::move( socket_sp ) ]( base::ErrorCode const &err ) mutable {
								    if( static_cast<bool>( err ) ) {
									    std::cerr << "async_accept: ERROR: " << err << ": " << err.message( ) << "\n\n";
								    }
								    handle_accept( obj, socket_sp, err );
							    };
							    m_acceptor->async_accept( boost_socket->lowest_layer( ), async_accept_handler );
						    } );
					}

					void NetSslServerImpl::emit_connection( NetSocketStream socket ) {
						emitter( )->emit( "connection", std::move( socket ) );
					}

					void NetSslServerImpl::emit_listening( EndPoint endpoint ) {
						emitter( )->emit( "listening", std::move( endpoint ) );
					}

					void NetSslServerImpl::emit_closed( ) {
						emitter( )->emit( "closed" );
					}
				} // namespace impl

				std::string SSLConfig::get_tls_ca_verify_file( ) const {
					if( tls_ca_verify_file.empty( ) ) {
						return tls_ca_verify_file;
					}
					boost::filesystem::path p{tls_ca_verify_file};
					return canonical( p ).string( );
				}

				std::string SSLConfig::get_tls_certificate_chain_file( ) const {
					if( tls_certificate_chain_file.empty( ) ) {
						return tls_certificate_chain_file;
					}
					boost::filesystem::path p{tls_certificate_chain_file};
					return canonical( p ).string( );
				}

				std::string SSLConfig::get_tls_private_key_file( ) const {
					if( tls_private_key_file.empty( ) ) {
						return tls_private_key_file;
					}
					boost::filesystem::path p{tls_private_key_file};
					return canonical( p ).string( );
				}

				std::string SSLConfig::get_tls_dh_file( ) const {
					if( tls_dh_file.empty( ) ) {
						return tls_dh_file;
					}
					boost::filesystem::path p{tls_dh_file};
					return canonical( p ).string( );
				}

				void SSLConfig::json_link_map( ) {
					link_json_string( "tls_ca_verify_file", tls_ca_verify_file );
					link_json_string( "tls_certificate_chain_file", tls_certificate_chain_file );
					link_json_string( "tls_private_key_file", tls_private_key_file );
					link_json_string( "tls_dh_file", tls_dh_file );
				}
			} // namespace net
		}     // namespace lib
	}         // namespace nodepp
} // namespace daw
