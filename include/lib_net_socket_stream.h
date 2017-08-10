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

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <memory>
#include <string>

#include <daw/daw_string_view.h>

#include "base_enoding.h"
#include "base_error.h"
#include "base_selfdestruct.h"
#include "base_semaphore.h"
#include "base_service_handle.h"
#include "base_stream.h"
#include "base_types.h"
#include "base_write_buffer.h"
#include "lib_net_dns.h"
#include "lib_net_socket_boost_socket.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace net {
				namespace impl {
					struct NetSocketStreamImpl;
				}

				using NetSocketStream = std::shared_ptr<impl::NetSocketStreamImpl>;

				NetSocketStream create_net_socket_stream( std::shared_ptr<boost::asio::ssl::context> context );
				NetSocketStream create_net_socket_stream( );
				NetSocketStream create_net_socket_stream( boost::asio::ssl::context::method method );

				enum class NetSocketStreamReadMode : uint8_t {
					newline,
					buffer_full,
					predicate,
					next_byte,
					regex,
					values,
					double_newline
				};

				namespace impl {
					struct NetSocketStreamImpl
					    : public daw::nodepp::base::SelfDestructing<NetSocketStreamImpl>,
					      public daw::nodepp::base::stream::StreamReadableEvents<NetSocketStreamImpl>,
					      public daw::nodepp::base::stream::StreamWritableEvents<NetSocketStreamImpl> {
						using match_iterator_t =
						    boost::asio::buffers_iterator<base::stream::StreamBuf::const_buffers_type>;
						using match_function_t = std::function<std::pair<match_iterator_t, bool>(
						    match_iterator_t begin, match_iterator_t end )>;

					  private:
						BoostSocket m_socket;

						struct netsockstream_state_t {
							bool closed = false;
							bool end = false;
							netsockstream_state_t( ) = default;
							~netsockstream_state_t( ) = default;
							netsockstream_state_t( netsockstream_state_t const & ) = default;
							netsockstream_state_t( netsockstream_state_t && ) noexcept = default;
							netsockstream_state_t &operator=( netsockstream_state_t const & ) = default;
							netsockstream_state_t &operator=( netsockstream_state_t && ) noexcept = default;
						} m_state;

						struct netsockstream_readoptions_t {
							NetSocketStreamReadMode read_mode = NetSocketStreamReadMode::newline;
							size_t max_read_size = 8192;
							std::unique_ptr<NetSocketStreamImpl::match_function_t> read_predicate;
							std::string read_until_values;

							netsockstream_readoptions_t( ) = default;

							~netsockstream_readoptions_t( ) = default;

							explicit netsockstream_readoptions_t( size_t max_read_size_ )
							    : read_mode{NetSocketStreamReadMode::newline}, max_read_size{max_read_size_} {}

							netsockstream_readoptions_t( netsockstream_readoptions_t const & ) = delete;
							netsockstream_readoptions_t( netsockstream_readoptions_t && ) noexcept = default;
							netsockstream_readoptions_t &operator=( netsockstream_readoptions_t const & ) = delete;
							netsockstream_readoptions_t &operator=( netsockstream_readoptions_t && ) noexcept = default;
						} m_read_options;

						struct ssl_params_t {
							void set_verify_mode( );
							void set_verify_callback( );
						};

						std::shared_ptr<daw::nodepp::base::Semaphore<int>> m_pending_writes;
						daw::nodepp::base::data_t m_response_buffers;
						std::size_t m_bytes_read;
						std::size_t m_bytes_written;

						explicit NetSocketStreamImpl( base::EventEmitter emitter );
						NetSocketStreamImpl( std::shared_ptr<boost::asio::ssl::context> ctx,
						                     base::EventEmitter emitter );

					  public:
						static NetSocketStream create( );
						static NetSocketStream create( std::shared_ptr<boost::asio::ssl::context> context );
						static NetSocketStream create( boost::asio::ssl::context::method method );

						~NetSocketStreamImpl( ) override;
						NetSocketStreamImpl( NetSocketStreamImpl const & ) = delete;
						NetSocketStreamImpl( NetSocketStreamImpl && ) = default;
						NetSocketStreamImpl &operator=( NetSocketStreamImpl const & ) = delete;
						NetSocketStreamImpl &operator=( NetSocketStreamImpl && ) = default;

						NetSocketStreamImpl &
						read_async( std::shared_ptr<daw::nodepp::base::stream::StreamBuf> read_buffer = nullptr );
						daw::nodepp::base::data_t read( );
						daw::nodepp::base::data_t read( std::size_t bytes );

						NetSocketStreamImpl &async_write( daw::nodepp::base::data_t const &chunk );
						NetSocketStreamImpl &
						write_async( daw::string_view chunk,
						             daw::nodepp::base::Encoding const &encoding = daw::nodepp::base::Encoding( ) );

						NetSocketStreamImpl &write( base::data_t const &chunk );
						NetSocketStreamImpl &write( string_view chunk, base::Encoding const & );
						NetSocketStreamImpl &write_from_file( string_view file_name );
						NetSocketStreamImpl &async_write_from_file( string_view file_name );

						NetSocketStreamImpl &end( );
						NetSocketStreamImpl &end( daw::nodepp::base::data_t const &chunk );
						NetSocketStreamImpl &
						end( daw::string_view chunk,
						     daw::nodepp::base::Encoding const &encoding = daw::nodepp::base::Encoding( ) );

						NetSocketStreamImpl &connect( daw::string_view host, uint16_t port );

						void close( bool emit_cb = true );
						void cancel( );

						bool is_open( ) const;
						bool is_closed( ) const;
						bool can_write( ) const;

						NetSocketStreamImpl &set_read_mode( NetSocketStreamReadMode mode );
						NetSocketStreamReadMode const &current_read_mode( ) const;
						NetSocketStreamImpl &
						set_read_predicate( std::function<std::pair<NetSocketStreamImpl::match_iterator_t, bool>(
						                        NetSocketStreamImpl::match_iterator_t begin,
						                        NetSocketStreamImpl::match_iterator_t end )>
						                        match_function );
						NetSocketStreamImpl &clear_read_predicate( );
						NetSocketStreamImpl &set_read_until_values( std::string values, bool is_regex );

						daw::nodepp::lib::net::impl::BoostSocket &socket( );
						daw::nodepp::lib::net::impl::BoostSocket const &socket( ) const;

						std::size_t &buffer_size( );

						NetSocketStreamImpl &set_timeout( int32_t value );

						NetSocketStreamImpl &set_no_delay( bool noDelay );
						NetSocketStreamImpl &set_keep_alive( bool keep_alive, int32_t initial_delay );

						std::string remote_address( ) const;
						std::string local_address( ) const;
						uint16_t remote_port( ) const;
						uint16_t local_port( ) const;

						std::size_t bytes_read( ) const;
						std::size_t bytes_written( ) const;

						//////////////////////////////////////////////////////////////////////////
						/// Callbacks

						//////////////////////////////////////////////////////////////////////////
						/// Summary: Event emitted when a connection is established
						NetSocketStreamImpl &on_connected( std::function<void( NetSocketStream )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// Summary: Event emitted when a connection is established
						NetSocketStreamImpl &on_next_connected( std::function<void( NetSocketStream )> listener );

						//////////////////////////////////////////////////////////////////////////
						/// StreamReadable

						void emit_connect( );
						void emit_timeout( );

					  private:
						static void handle_connect( std::weak_ptr<NetSocketStreamImpl> obj,
						                            base::ErrorCode const &err );

						static void handle_read( std::weak_ptr<NetSocketStreamImpl> obj,
						                         std::shared_ptr<daw::nodepp::base::stream::StreamBuf> read_buffer,
						                         base::ErrorCode const &err, std::size_t const &bytes_transferred );
						static void handle_write( std::weak_ptr<daw::nodepp::base::Semaphore<int>> outstanding_writes,
						                          std::weak_ptr<NetSocketStreamImpl> obj,
						                          daw::nodepp::base::write_buffer buff, base::ErrorCode const &err,
						                          size_t const &bytes_transferred );

						static void handle_write( std::weak_ptr<daw::nodepp::base::Semaphore<int>> outstanding_writes,
						                          std::weak_ptr<NetSocketStreamImpl> obj, base::ErrorCode const &err,
						                          size_t const &bytes_transfered );

						void async_write( daw::nodepp::base::write_buffer buff );

						void write( base::write_buffer buff );

					}; // struct NetSocketStreamImpl
				}      // namespace impl

				//	daw::nodepp::lib::net::NetSocketStream& operator<<( daw::nodepp::lib::net::NetSocketStream socket,
				// daw::string_view message );
			} // namespace net
		}     // namespace lib
	}         // namespace nodepp
} // namespace daw
daw::nodepp::lib::net::NetSocketStream &operator<<( daw::nodepp::lib::net::NetSocketStream &socket,
                                                    daw::string_view message );
