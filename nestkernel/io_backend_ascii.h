/*
 *  io_backend_ascii.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef IO_BACKEND_ASCII_H
#define IO_BACKEND_ASCII_H

#include <map>
#include <fstream>

#include "io_backend.h"

namespace nest
{

/**
 * ASCII specialization of the Logger interface.
 * Recorded data is written to plain text files on a per-device-per-thread basis.
 * Some formatting options are available to allow some compatibility to legacy NEST output files.
 *
 * IOBackendASCII maintains a data structure mapping one file stream to every recording device instance
 * on every thread. Files are opened and inserted into the map during the initialize() call and
 * closed in finalize().
 */
class IOBackendASCII : public IOBackend
{
public:
  /**
   * IOBackendASCII constructor.
   * The actual setup is done in initialize().
   */
  IOBackendASCII()
    : files_()
  {
  }

  /**
   * IOBackendASCII descructor.
   * File handling is done in finalize().
   */
  ~IOBackendASCII() throw()
  {
    // remaining files are not closed here but should be handled gracefully on NEST shutdown.
  }

  /**
   * Functions called by all instantiated recording devices to register themselves with their
   * metadata.
   */
  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device, const std::vector< Name >& value_names );

  /**
   * Initialize the IOBackendASCII during simulation preparation. Here, files are opened for all
   * previously enrolled devices.
   */
  void initialize();

  /**
   * Finalize the IOBackendASCII after the simulation has finished. Files are flushed and/or closed
   * according to `close_after_simulate` and `flush_after_simulate` parameters.
   */
  void finalize();

  /**
   * Trivial synchronization function. The IOBackendASCII does not need explicit synchronization after
   * each time step.
   */
  void synchronize();

  /**
   * Functions to write data to file.
   */
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device, const Event& event, const std::vector< double_t >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

private:
  /**
   * Build filename from parts.
   * The filename consists of the data path set in IOManager, the devices' labels (or names as a
   * fallback if no label is given), the device GID, and the virtual process ID, all separated by
   * dashes.
   */
  const std::string build_filename_( const RecordingDevice& device ) const;

  struct Parameters_
  {
    long precision_;

    std::string file_ext_;      //!< the file name extension to use, without .
    long fbuffer_size_;         //!< the buffer size to use when writing to file
    long fbuffer_size_old_;     //!< the buffer size to use when writing to file (old)
    bool close_after_simulate_; //!< if true, finalize() shall close the stream
    bool flush_after_simulate_; //!< if true, finalize() shall flush the stream

    Parameters_();

    void get( const IOBackendASCII&, DictionaryDatum& ) const;
    void set( const IOBackendASCII&, const DictionaryDatum& );
  };

  Parameters_ P_;

  // one map for each virtual process,
  // in turn containing one ostream for everydevice
  // vp -> (gid -> [device, filestream])
  typedef std::map< int, std::map< int, std::pair< RecordingDevice*, std::ofstream* > > > file_map;
  file_map files_;
};

inline void
IOBackendASCII::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // IO_BACKEND_ASCII_H