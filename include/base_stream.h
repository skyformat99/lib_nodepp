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

#include <boost/asio/streambuf.hpp>
#include <functional>

#include "base_event_emitter.h"
#include "base_types.h"

namespace daw {
	namespace nodepp {
		namespace base {
			namespace stream {
				using StreamBuf = boost::asio::streambuf;

				template<typename Derived>
				class StreamWritableEvents {
					Derived &derived( ) noexcept {
						return *static_cast<Derived *>( this );
					}

					Derived const &derived( ) const noexcept {
						return *static_cast<Derived const *>( this );
					}

					auto &derived_emitter( ) noexcept( noexcept( std::declval<Derived>( ).emitter( ) ) ) {
						return derived( ).emitter( );
					}

					auto const &derived_emitter( ) const noexcept( noexcept( std::declval<Derived>( ).emitter( ) ) ) {
						return derived( ).emitter( );
					}

				  protected:
					constexpr StreamWritableEvents( ) noexcept = default;
					~StreamWritableEvents( ) = default;
					constexpr StreamWritableEvents( StreamWritableEvents const & ) noexcept = default;
					constexpr StreamWritableEvents( StreamWritableEvents && ) noexcept = default;
					constexpr StreamWritableEvents &operator=( StreamWritableEvents const & ) noexcept = default;
					constexpr StreamWritableEvents &operator=( StreamWritableEvents && ) noexcept = default;

				  public:
					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when a pending write is completed
					Derived &on_write_completion( std::function<void( std::shared_ptr<Derived> )> listener ) {
						derived_emitter( )->add_listener( "write_completion", listener );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when the next pending write is completed
					Derived &on_next_write_completion( std::function<void( std::shared_ptr<Derived> )> listener ) {
						derived_emitter( )->add_listener( "write_completion", listener );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when end( ... ) has been called and all
					///				data has been flushed
					Derived &on_all_writes_completed( std::function<void( std::shared_ptr<Derived> )> listener ) {
						derived_emitter( )->add_listener( "all_writes_completed", listener );
						return derived( );
					}

					Derived &close_when_writes_completed( ) {
						struct {
							void operator( )( std::shared_ptr<Derived> resp ) {
								resp->close( false );
							}
						} handler;
						derived_emitter( )->add_listener( "all_writes_completed", handler );
						return derived( );
					}
					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when an async write completes
					void emit_write_completion( std::shared_ptr<Derived> obj ) {
						derived_emitter( )->emit( "write_completion", std::move( obj ) );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	All async writes have completed
					void emit_all_writes_completed( std::shared_ptr<Derived> obj ) {
						derived_emitter( )->emit( "all_writes_completed", std::move( obj ) );
					}
				}; // class StreamWritableEvents

				template<typename Derived>
				class StreamReadableEvents {
					Derived &derived( ) noexcept {
						return *static_cast<Derived *>( this );
					}

					Derived const &derived( ) const noexcept {
						return *static_cast<Derived const *>( this );
					}

					auto &derived_emitter( ) noexcept( noexcept( std::declval<Derived>( ).emitter( ) ) ) {
						return derived( ).emitter( );
					}

					auto const &derived_emitter( ) const noexcept( noexcept( std::declval<Derived>( ).emitter( ) ) ) {
						return derived( ).emitter( );
					}

				  protected:
					constexpr StreamReadableEvents( ) noexcept = default;
					~StreamReadableEvents( ) = default;
					constexpr StreamReadableEvents( StreamReadableEvents const & ) noexcept = default;
					constexpr StreamReadableEvents( StreamReadableEvents && ) noexcept = default;
					constexpr StreamReadableEvents &operator=( StreamReadableEvents const & ) noexcept = default;
					constexpr StreamReadableEvents &operator=( StreamReadableEvents && ) noexcept = default;

				  public:
					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when data is received
					Derived &on_data_received(
					    std::function<void( std::shared_ptr<base::data_t> buffer, bool end_of_file )> listener ) {
						derived_emitter( )->add_listener( "data_received", listener );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when data is received
					Derived &on_next_data_received(
					    std::function<void( std::shared_ptr<base::data_t> buffer, bool end_of_file )> listener ) {
						derived_emitter( )->add_listener( "data_received", listener, true );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when of of stream is read.
					Derived &on_eof( std::function<void( std::shared_ptr<Derived> )> listener ) {
						derived_emitter( )->add_listener( "eof", listener );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when of of stream is read.
					Derived &on_next_eof( std::function<void( std::shared_ptr<Derived> )> listener ) {
						derived_emitter( )->add_listener( "eof", listener, true );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Event emitted when the stream is closed
					Derived &on_closed( std::function<void( std::shared_ptr<Derived> )> listener ) {
						derived_emitter( )->add_listener( "closed", listener );
						return derived( );
					}

					Derived &on_closed( std::function<void( )> listener ) {
						derived_emitter( )->add_listener( "closed", listener );
						return derived( );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary:	Emit an event with the data received and whether the eof
					///				has been reached
					void emit_data_received( std::shared_ptr<daw::nodepp::base::data_t> buffer, bool end_of_file ) {
						derived_emitter( )->emit( "data_received", std::move( buffer ), end_of_file );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary: Event emitted when the eof has been reached
					void emit_eof( ) {
						derived_emitter( )->emit( "eof" );
					}

					//////////////////////////////////////////////////////////////////////////
					/// Summary: Event emitted when the socket is closed
					void emit_closed( ) {
						derived_emitter( )->emit( "closed" );
					}

					template<typename StreamWritableObj>
					Derived &delegate_data_received_to( std::weak_ptr<StreamWritableObj> stream_writable_obj ) {
						on_data_received( [stream_writable_obj]( base::data_t buff, bool eof ) {
							if( !stream_writable_obj.expired( ) ) {
								stream_writable_obj.lock( )->write( buff );
							}
						} );
						return derived( );
					}
				}; // class StreamReadableEvents
			}      // namespace stream
		}          // namespace base
	}              // namespace nodepp
} // namespace daw
