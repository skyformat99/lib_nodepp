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

#include <boost/shared_ptr.hpp>
#include <cinttypes>
#include <cstdlib>
#include <iterator>
#include <string>
#include <utility>

#include <daw/daw_exception.h>
#include <daw/daw_range_algorithm.h>
#include <daw/daw_utility.h>

#include "base_event_emitter.h"
#include "base_service_handle.h"
#include "lib_http_connection.h"
#include "lib_http_server.h"
#include "lib_net_server.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace http {
				namespace impl {
					using namespace daw::nodepp;

					HttpServerImpl::HttpServerImpl( base::EventEmitter emitter )
					    : daw::nodepp::base::StandardEvents<HttpServerImpl>{std::move( emitter )}
					    , m_netserver{lib::net::create_net_server( )} {}

					HttpServerImpl::HttpServerImpl( daw::nodepp::lib::net::SslServerConfig const &ssl_config,
					                                daw::nodepp::base::EventEmitter emitter )
					    : daw::nodepp::base::StandardEvents<HttpServerImpl>{std::move( emitter )}
					    , m_netserver{lib::net::create_net_server( ssl_config )} {}

					void HttpServerImpl::emit_client_connected( HttpServerConnection connection ) {
						emitter( )->emit( "client_connected", std::move( connection ) );
					}

					void HttpServerImpl::emit_closed( ) {
						emitter( )->emit( "closed" );
					}

					void HttpServerImpl::emit_listening( daw::nodepp::lib::net::EndPoint endpoint ) {
						emitter( )->emit( "listening", std::move( endpoint ) );
					}

					void HttpServerImpl::handle_connection( std::weak_ptr<HttpServerImpl> obj,
					                                        lib::net::NetSocketStream socket ) {
						run_if_valid(
						    obj, "Exception while connecting", "HttpServerImpl::handle_connection",
						    [ obj, socket = std::move( socket ) ]( HttpServer self ) mutable {
							    auto connection = create_http_server_connection( std::move( socket ) );
							    auto it = self->m_connections.emplace( self->m_connections.end( ), connection );

							    connection->on_error( self, "Connection Error", "HttpServerImpl::handle_connection" )
							        .on_closed( [it, obj]( ) mutable {
								        if( !obj.expired( ) ) {
									        auto self_l = obj.lock( );
									        try {
										        self_l->m_connections.erase( it );
									        } catch( ... ) {
										        self_l->emit_error( std::current_exception( ),
										                            "Could not delete connection",
										                            "HttpServerImpl::handle_connection" );
									        }
								        }
							        } )
							        .start( );

							    try {
								    self->emit_client_connected( std::move( connection ) );
							    } catch( ... ) {
								    self->emit_error( std::current_exception( ), "Running connection listeners",
								                      "HttpServerImpl::handle_connection" );
							    }
						    } );
					}

					void HttpServerImpl::listen_on( uint16_t port, daw::nodepp::lib::net::ip_version ip_ver,
					                                uint16_t max_backlog ) {
						emit_error_on_throw( get_ptr( ), "Error while listening", "HttpServerImpl::listen_on", [&]( ) {
							auto obj = this->get_weak_ptr( );
							m_netserver
							    ->on_connection( [obj]( lib::net::NetSocketStream socket ) {
								    handle_connection( obj, std::move( socket ) );
							    } )
							    .on_error( obj, "Error listening", "HttpServerImpl::listen_on" )
							    .template delegate_to<daw::nodepp::lib::net::EndPoint>( "listening", obj, "listening" )
							    .listen( port, ip_ver, max_backlog );
						} );
					}

					size_t &HttpServerImpl::max_header_count( ) {
						daw::exception::daw_throw_not_implemented( );
					}

					size_t const &HttpServerImpl::max_header_count( ) const {
						daw::exception::daw_throw_not_implemented( );
					}

					size_t HttpServerImpl::timeout( ) const {
						daw::exception::daw_throw_not_implemented( );
					}

					HttpServerImpl &
					HttpServerImpl::on_listening( std::function<void( daw::nodepp::lib::net::EndPoint )> listener ) {
						emitter( )->add_listener( "listening", std::move( listener ) );
						return *this;
					}

					HttpServerImpl &HttpServerImpl::on_next_listening(
					    std::function<void( daw::nodepp::lib::net::EndPoint )> listener ) {
						emitter( )->add_listener( "listening", std::move( listener ), true );
						return *this;
					}

					///
					/// \param listener - a callback that takes a HttpServerConnection as it's argument
					/// \return - a reference to *this
					HttpServerImpl &
					HttpServerImpl::on_client_connected( std::function<void( HttpServerConnection )> listener ) {
						emitter( )->add_listener( "client_connected", std::move( listener ) );
						return *this;
					}

					HttpServerImpl &
					HttpServerImpl::on_next_client_connected( std::function<void( HttpServerConnection )> listener ) {
						emitter( )->add_listener( "client_connected", std::move( listener ), true );
						return *this;
					}

					HttpServerImpl &HttpServerImpl::on_closed( std::function<void( )> listener ) {
						emitter( )->add_listener( "closed", std::move( listener ) );
						return *this;
					}

					HttpServerImpl &HttpServerImpl::on_next_closed( std::function<void( )> listener ) {
						emitter( )->add_listener( "closed", std::move( listener ), true );
						return *this;
					}

					HttpServerImpl::~HttpServerImpl( ) = default;
				} // namespace impl

				HttpServer create_http_server( daw::nodepp::base::EventEmitter emitter ) {

					auto result = new impl::HttpServerImpl{std::move( emitter )};
					return HttpServer{result};
				}

				HttpServer create_http_server( daw::nodepp::lib::net::SslServerConfig const &ssl_config,
				                               daw::nodepp::base::EventEmitter emitter ) {

					auto result = new impl::HttpServerImpl{ssl_config, std::move( emitter )};
					return HttpServer{result};
				}
			} // namespace http
		}     // namespace lib
	}         // namespace nodepp
} // namespace daw
