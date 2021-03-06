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

#include <boost/variant.hpp>

#include "lib_net_nossl_server.h"
#include "lib_net_ssl_server.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace net {
				namespace impl {
					class NetServerImpl;
				}
				using NetServer = std::shared_ptr<impl::NetServerImpl>;

				NetServer create_net_server( daw::nodepp::base::EventEmitter emitter =
				                                                    daw::nodepp::base::create_event_emitter( ) );

				NetServer create_net_server(
				    daw::nodepp::lib::net::SslServerConfig const &ssl_config,
				    daw::nodepp::base::EventEmitter emitter = daw::nodepp::base::create_event_emitter( ) );

				namespace impl {
					//////////////////////////////////////////////////////////////////////////
					// Summary:		A TCP Server class
					// Requires:	daw::nodepp::base::EventEmitter, daw::nodepp::base::options_t,
					//				daw::nodepp::lib::net::NetAddress, daw::nodepp::base::Error
					class NetServerImpl : public daw::nodepp::base::enable_shared<NetServerImpl>,
					                      public daw::nodepp::base::StandardEvents<NetServerImpl> {
						using NetNoSslServer = std::shared_ptr<NetNoSslServerImpl>;
						using NetSslServer = std::shared_ptr<NetSslServerImpl>;
						using value_type = boost::variant<NetNoSslServer, NetSslServer>;
						value_type m_net_server;

						NetServerImpl( daw::nodepp::base::EventEmitter emitter );

						NetServerImpl( daw::nodepp::lib::net::SslServerConfig const &ssl_config,
						               daw::nodepp::base::EventEmitter emitter );

						friend daw::nodepp::lib::net::NetServer
						daw::nodepp::lib::net::create_net_server( daw::nodepp::base::EventEmitter emitter );

						friend daw::nodepp::lib::net::NetServer daw::nodepp::lib::net::create_net_server(
						    daw::nodepp::lib::net::SslServerConfig const &ssl_config,
						    daw::nodepp::base::EventEmitter emitter );

					  public:
						NetServerImpl( ) = delete;
						~NetServerImpl( ) override;
						NetServerImpl( NetServerImpl const & ) = default;
						NetServerImpl( NetServerImpl && ) noexcept = default;
						NetServerImpl &operator=( NetServerImpl const & ) = default;
						NetServerImpl &operator=( NetServerImpl && ) noexcept = default;

						bool using_ssl( ) const noexcept;

						void listen( uint16_t port, ip_version ip_ver = ip_version::ipv4_v6, uint16_t max_backlog = 511 );

						void close( );

						daw::nodepp::lib::net::NetAddress const &address( ) const;

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
					}; // class NetServerImpl
				}      // namespace impl

			} // namespace net
		}     // namespace lib
	}         // namespace nodepp
} // namespace daw
