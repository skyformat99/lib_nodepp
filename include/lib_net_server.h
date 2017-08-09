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

#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <list>
#include <memory>
#include <string>

#include <daw/json/daw_json_link.h>

#include "base_error.h"
#include "base_event_emitter.h"
#include "base_service_handle.h"
#include "base_types.h"
#include "lib_net_address.h"
#include "lib_net_socket_stream.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace net {
				namespace impl {
					class NetServerImpl;
				}

				struct SSLConfig : public daw::json::daw_json_link<SSLConfig> {
					std::string tls_ca_verify_file;
					std::string tls_certificate_chain_file;
					std::string tls_private_key_file;
					std::string tls_dh_file;

					static void json_link_map( ) {
						link_json_string( "tls_ca_verify_file", tls_ca_verify_file );
						link_json_string( "tls_certificate_chain_file", tls_certificate_chain_file );
						link_json_string( "tls_private_key_file", tls_private_key_file );
						link_json_string( "tls_dh_file", tls_dh_file );
					}

					std::string get_tls_ca_verify_file( ) const;
					std::string get_tls_certificate_chain_file( ) const;
					std::string get_tls_private_key_file( ) const;
					std::string get_tls_dh_file( ) const;
				};

				using NetServer = std::shared_ptr<impl::NetServerImpl>;
				using EndPoint = boost::asio::ip::tcp::endpoint;

				NetServer create_net_server( );
				NetServer create_net_server( daw::nodepp::base::EventEmitter emitter );
				NetServer create_net_server(
				    boost::asio::ssl::context::method ctx_method,
				    daw::nodepp::base::EventEmitter emitter = daw::nodepp::base::create_event_emitter( ) );

				NetServer create_net_server(
				    daw::nodepp::lib::net::SSLConfig const &ssl_config,
				    daw::nodepp::base::EventEmitter emitter = daw::nodepp::base::create_event_emitter( ) );
				namespace impl {
					//////////////////////////////////////////////////////////////////////////
					// Summary:		A TCP Server class
					// Requires:	daw::nodepp::base::EventEmitter, daw::nodepp::base::options_t,
					//				daw::nodepp::lib::net::NetAddress, daw::nodepp::base::Error
					class NetServerImpl : public daw::nodepp::base::enable_shared<NetServerImpl>,
					                      public daw::nodepp::base::StandardEvents<NetServerImpl> {
						std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
						std::shared_ptr<boost::asio::ssl::context> m_context;

						explicit NetServerImpl( daw::nodepp::base::EventEmitter emitter );
						NetServerImpl( boost::asio::ssl::context::method method,
						               daw::nodepp::base::EventEmitter emitter );

						NetServerImpl( daw::nodepp::lib::net::SSLConfig const &ssl_config,
						               daw::nodepp::base::EventEmitter emitter );

					  public:
						static NetServer create( );
						static NetServer create( daw::nodepp::base::EventEmitter emitter );
						static NetServer create( boost::asio::ssl::context::method ctx_method,
						                         daw::nodepp::base::EventEmitter emitter );

						static NetServer create( daw::nodepp::lib::net::SSLConfig const &ssl_config,
						                         daw::nodepp::base::EventEmitter emitter );

						NetServerImpl( ) = delete;
						~NetServerImpl( );
						NetServerImpl( NetServerImpl const & ) = default;
						NetServerImpl( NetServerImpl && ) = default;
						NetServerImpl &operator=( NetServerImpl const & ) = default;
						NetServerImpl &operator=( NetServerImpl && ) = default;

						boost::asio::ssl::context &ssl_context( );
						boost::asio::ssl::context const &ssl_context( ) const;
						bool using_ssl( ) const;

						void listen( uint16_t port );
						void listen( uint16_t port, std::string hostname, uint16_t backlog = 511 );
						void listen( std::string socket_path );
						void close( );

						daw::nodepp::lib::net::NetAddress const &address( ) const;

						void set_max_connections( uint16_t value );

						void
						get_connections( std::function<void( daw::nodepp::base::Error err, uint16_t count )> callback );

						// Event callbacks

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when a connection is established
						NetServerImpl &on_connection( std::function<void( NetSocketStream socket )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when a connection is established
						NetServerImpl &on_next_connection( std::function<void( NetSocketStream socket )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when the server is bound after calling
						/// listen( ... )
						NetServerImpl &on_listening( std::function<void( EndPoint )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when the server is bound after calling
						/// listen( ... )
						NetServerImpl &on_next_listening( std::function<void( )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when the server closes and all connections
						/// are closed
						NetServerImpl &on_closed( std::function<void( )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when a connection is established
						void emit_connection( NetSocketStream socket );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when the server is bound after calling
						///				listen( ... )
						void emit_listening( EndPoint endpoint );

						//////////////////////////////////////////////////////////////////////////
						/// Summary:	Event emitted when the server is bound after calling
						///				listen( ... )
						void emit_closed( );

					  private:
						static void handle_accept( std::weak_ptr<NetServerImpl> obj, NetSocketStream socket,
						                           base::ErrorCode const &err );

						void start_accept( );
					}; // class NetServerImpl
				}      // namespace impl
			}          // namespace net
		}              // namespace lib
	}                  // namespace nodepp
} // namespace daw
