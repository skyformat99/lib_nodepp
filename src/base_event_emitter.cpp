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

#include <boost/asio/error.hpp>
#include <memory>
#include <utility>
#include <vector>

#include <daw/daw_range_algorithm.h>
#include <daw/daw_string_view.h>

#include "base_event_emitter.h"

namespace daw {
	namespace nodepp {
		namespace base {
			namespace impl {
				EventEmitterImpl::EventEmitterImpl( size_t max_listeners )
				    : m_listeners{std::make_shared<listeners_t>( )}
				    , m_max_listeners{max_listeners}
				    , m_emit_depth{std::make_shared<std::atomic_int_least8_t>( 0 )}
				    , m_allow_cb_without_params{true} {}

				EventEmitterImpl::listeners_t &EventEmitterImpl::listeners( ) {
					return *m_listeners;
				}

				bool EventEmitterImpl::operator==( EventEmitterImpl const &rhs ) const noexcept {
					return this == &rhs; // All we need is a pointer comparison
				}

				bool EventEmitterImpl::operator!=( EventEmitterImpl const &rhs ) const noexcept {
					return this != &rhs;
				}

				bool EventEmitterImpl::at_max_listeners( daw::string_view event ) {
					auto result = 0 != m_max_listeners;
					result &= listeners( )[event.to_string( )].size( ) >= m_max_listeners;
					return result;
				}

				void EventEmitterImpl::remove_listener( daw::string_view event, callback_id_t id ) {
					daw::algorithm::erase_remove_if( listeners( )[event.to_string( )],
					                                 [&]( std::pair<bool, Callback> const &item ) {
						                                 if( item.second.id( ) == id ) {
							                                 // TODO: verify if this needs to be outside loop
							                                 emit_listener_removed( event, item.second );
							                                 return true;
						                                 }
						                                 return false;
					                                 } );
				}

				void EventEmitterImpl::remove_listener( daw::string_view event, Callback listener ) {
					return remove_listener( event, listener.id( ) );
				}

				void EventEmitterImpl::remove_all_listeners( ) {
					listeners( ).clear( );
				}

				void EventEmitterImpl::remove_all_listeners( daw::string_view event ) {
					listeners( )[event.to_string( )].clear( );
				}

				void EventEmitterImpl::set_max_listeners( size_t max_listeners ) {
					m_max_listeners = max_listeners;
				}

				EventEmitterImpl::listener_list_t EventEmitterImpl::listeners( daw::string_view event ) {
					return listeners( )[event.to_string( )];
				}

				size_t EventEmitterImpl::listener_count( daw::string_view event ) {
					return listeners( event ).size( );
				}

				void EventEmitterImpl::emit_listener_added( daw::string_view event, Callback listener ) {
					emit( "listener_added", event, std::move( listener ) );
				}

				void EventEmitterImpl::emit_listener_removed( daw::string_view event, Callback listener ) {
					emit( "listener_removed", event, std::move( listener ) );
				}

				EventEmitterImpl::~EventEmitterImpl( ) = default;

				EventEmitter EventEmitterImpl::create( size_t max_listeners ) noexcept {
					try {
						auto result = new EventEmitterImpl{max_listeners};
						return EventEmitter{result};
					} catch( ... ) { return EventEmitter{nullptr}; }
				}
			} // namespace impl

			EventEmitter create_event_emitter( size_t max_listeners ) noexcept {
				return impl::EventEmitterImpl::create( max_listeners );
			}
		} // namespace base
	}     // namespace nodepp
} // namespace daw
