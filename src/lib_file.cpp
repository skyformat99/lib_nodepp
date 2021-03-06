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

#include <fstream>

#include <daw/daw_string_view.h>
#include <daw/daw_utility.h>

#include "base_task_management.h"
#include "lib_file.h"

namespace daw {
	namespace nodepp {
		namespace lib {
			namespace file {
				namespace {
					std::streampos file_size( std::ifstream &stream ) {
						if( !stream ) {
							return -1;
						}
						auto current_pos = stream.tellg( );
						if( current_pos < 0 ) {
							return -1;
						}
						stream.seekg( 0, std::ifstream::end );
						auto result = stream.tellg( );
						stream.seekg( current_pos );
						return result;
					}
				} // namespace

				std::streampos file_size( daw::string_view path ) {
					std::ifstream in_file( path.to_string( ), std::ifstream::ate | std::ifstream::binary );
					return file_size( in_file );
				}

				base::OptionalError read_file( daw::string_view path, base::data_t &buffer, bool append_buffer ) {
					std::ifstream in_file( path.to_string( ), std::ifstream::ate | std::ifstream::binary );
					if( !in_file ) {
						auto result = base::create_optional_error( "Could not open file" );
						result->add( "where", "read_file#open" );
						return result;
					}

					auto fsize = in_file.tellg( );
					if( fsize < 0 ) {
						auto result = base::create_optional_error( "Error reading file length" );
						result->add( "where", "read_file#tellg" );
						return result;
					}
					if( !in_file.seekg( 0 ) ) {
						auto result = base::create_optional_error( "Error reseting file position to beginning" );
						result->add( "where", "read_file#seekg" );
						return result;
					}

					size_t first_pos = append_buffer ? buffer.size( ) : 0;
					buffer.resize( first_pos + static_cast<size_t>( fsize ) );

					if( !in_file.read( buffer.data( ) + first_pos, fsize ) ) {
						auto result = base::create_optional_error( "Error reading file" );
						result->add( "where", "read_file#read" );
						return result;
					}
					return base::create_optional_error( );
				}

				base::OptionalError write_file( daw::string_view path, base::data_t const &buffer, FileWriteMode mode,
				                                size_t bytes_to_write ) {
					// TODO: Write to a temp file first and then move
					if( 0 == bytes_to_write || bytes_to_write > buffer.size( ) ) {
						bytes_to_write =
						    buffer.size( ); // TODO: verify this is what we want.  Covers errors but that may be OK
					}
					std::ofstream out_file;
					switch( mode ) {
					case FileWriteMode::AppendOrCreate:
						out_file.open( path.to_string( ),
						               std::ostream::binary | std::ostream::out | std::ostream::app );
						break;
					case FileWriteMode::MustCreate: {
						if( std::ifstream( path.to_string( ) ) ) {
							// File exists.  Error
							auto error = base::create_optional_error(
							    "Attempt to open an existing file when MustCreate requested" );
							error->add( "where", "write_from_file" );
							return error;
						}
						out_file.open( path.to_string( ), std::ostream::binary | std::ostream::out );
						break;
					}
					case FileWriteMode::OverwriteOrCreate:
						out_file.open( path.to_string( ),
						               std::ostream::binary | std::ostream::out | std::ostream::trunc );
						break;
					default:
						daw::exception::daw_throw_unexpected_enum( );
					}
					if( !out_file ) {
						auto error = base::create_optional_error( "Could not open file for writing" );
						error->add( "where", "write_from_file#open" );
						return error;
					}
					if( !out_file.write( buffer.data( ), static_cast<std::streamoff>( bytes_to_write ) ) ) {
						auto error = base::create_optional_error( "Error writing data to file" );
						error->add( "where", "write_from_file#write" );
						return error;
					}
					return base::create_optional_error( );
				}

				void read_file_async(
				    daw::string_view path,
				    std::function<void( base::OptionalError error, std::shared_ptr<base::data_t> data )> callback,
				    std::shared_ptr<base::data_t> buffer, bool append_buffer ) {

					auto task = [path, buffer, append_buffer]( ) mutable {
						if( !buffer ) {
							buffer.reset( new base::data_t{} );
						} else if( !append_buffer ) {
							buffer->resize( 0 );
						}
						return read_file( path, *buffer );
					};
					if( callback ) {
						auto when_task_completed = [buffer, callback]( base::OptionalError error ) mutable {
							callback( error, buffer );
						};
						base::add_task( task, when_task_completed );
					} else {
						base::add_task( task );
					}
				}

				void write_file_async( daw::string_view path, base::data_t buffer,
				                       std::function<void( base::OptionalError error )> callback, FileWriteMode mode,
				                       size_t bytes_to_write ) {

					auto task = [ path, buffer = std::move( buffer ), mode, bytes_to_write ]( ) mutable {
						return write_file( path, buffer, mode, bytes_to_write );
					};

					if( callback ) {
						auto when_task_completed = [callback =
						                                std::move( callback )]( base::OptionalError error ) mutable {
							callback( std::move( error ) );
						};
						base::add_task( task, when_task_completed );
					} else {
						base::add_task( task );
					}
				}
			} // namespace file
		}     // namespace lib
	}         // namespace nodepp
} // namespace daw
