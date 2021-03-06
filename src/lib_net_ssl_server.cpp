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

					NetSslServerImpl::NetSslServerImpl( daw::nodepp::lib::net::SslServerConfig ssl_config,
					                                    daw::nodepp::base::EventEmitter emitter )
					    : daw::nodepp::base::StandardEvents<NetSslServerImpl>{std::move( emitter )}
					    , m_acceptor{std::make_shared<boost::asio::ip::tcp::acceptor>( base::ServiceHandle::get( ) )}
					    , m_config{std::move( ssl_config )} {}

					NetSslServerImpl::~NetSslServerImpl( ) = default;

					void NetSslServerImpl::listen( uint16_t port, ip_version ip_ver, uint16_t max_backlog ) {
						emit_error_on_throw(
						    get_ptr( ), "Error listening for connection", "NetSslServerImpl::listen", [&]( ) {
							    auto const tcp = ip_ver == ip_version::ipv4 ? boost::asio::ip::tcp::v4( )
							                                                : boost::asio::ip::tcp::v6( );
							    auto endpoint = EndPoint( tcp, port );
							    m_acceptor->open( endpoint.protocol( ) );
							    m_acceptor->set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
							    set_ipv6_only( m_acceptor, ip_ver );
							    m_acceptor->bind( endpoint );
							    m_acceptor->listen( max_backlog );
							    start_accept( );
							    emitter( )->emit( "listening", std::move( endpoint ) );
						    } );
					}

					void NetSslServerImpl::close( ) {
						daw::exception::daw_throw_not_implemented( );
					}

					daw::nodepp::lib::net::NetAddress const &NetSslServerImpl::address( ) const {
						daw::exception::daw_throw_not_implemented( );
					}

					void NetSslServerImpl::get_connections(
					    std::function<void( base::Error err, uint16_t count )> callback ) {
						Unused( callback );
						daw::exception::daw_throw_not_implemented( );
					}

					void NetSslServerImpl::handle_handshake( std::weak_ptr<NetSslServerImpl> obj,
					                                         NetSocketStream socket, base::ErrorCode const &err ) {

						run_if_valid( std::move( obj ), "Error while handshaking", "NetSslServerImpl::handle_handshake", [socket=std::move(socket), &err](NetSslServer self ) {
							daw::exception::daw_throw_value_on_true( err );
							self->emitter( )->emit( "connection", socket );
						} );
					}

					void NetSslServerImpl::handle_accept( std::weak_ptr<NetSslServerImpl> obj, NetSocketStream socket,
					                                      base::ErrorCode const &err ) {
						run_if_valid(
						    std::move( obj ), "Exception while accepting connections",
						    "NetSslServerImpl::handle_accept",
						    [ socket, &err ]( NetSslServer self ) mutable {
								daw::exception::daw_throw_value_on_true( err );

							    socket->socket( ).async_handshake( boost::asio::ssl::stream_base::server, [
								    obj = self->get_weak_ptr( ), socket
							    ]( base::ErrorCode const &err1 ) mutable { handle_handshake( obj, socket, err1 ); } );
							    self->start_accept( );
						    } );
					}

					namespace {
						/*template<typename Handler>
						void async_accept( std::shared_ptr<boost::asio::ip::tcp::acceptor> &acceptor,
						                   boost::asio::ip::tcp::socket &socket, Handler handler ) {
						    acceptor->async_accept( socket, handler );
						}

						template<typename Handler>
						void async_accept( std::shared_ptr<boost::asio::ip::tcp::acceptor> &acceptor,
						                   boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &socket,
						                   Handler handler ) {
						    acceptor->async_accept( socket.next_layer( ), handler );
						}*/
					} // namespace

					void NetSslServerImpl::start_accept( ) {
						emit_error_on_throw(
						    get_ptr( ), "Error while starting accept", "NetSslServerImpl::start_accept", [&]( ) {
							    auto socket_sp = daw::nodepp::lib::net::create_net_socket_stream( m_config );
							    daw::exception::daw_throw_on_false(
							        socket_sp, "NetSslServerImpl::start_accept( ), Invalid socket - null" );

							    socket_sp->socket( ).init( );
							    auto &boost_socket = socket_sp->socket( );
							    m_acceptor->async_accept( boost_socket->lowest_layer( ), [
								    obj = this->get_weak_ptr( ), socket_sp
							    ]( base::ErrorCode const &err ) mutable {
								    daw::exception::daw_throw_value_on_true( err );
								    handle_accept( obj, socket_sp, err );
							    } );
						    } );
					}
				} // namespace impl

			} // namespace net
		}     // namespace lib
	}         // namespace nodepp
} // namespace daw
