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

namespace daw {
	namespace nodepp {
		namespace base {
			using IoService = boost::asio::io_service;

			struct ServiceHandle {
				static IoService &get( );

				static void run( );

				static void stop( );

				static void reset( );

				static void work( );

			  protected:
				ServiceHandle( ) = default;
			}; // struct ServiceHandle

			enum class StartServiceMode : uint_fast8_t { Single, OnePerCore };

			void
			start_service( daw::nodepp::base::StartServiceMode mode = daw::nodepp::base::StartServiceMode::Single );
		} // namespace base
	}     // namespace nodepp
} // namespace daw
