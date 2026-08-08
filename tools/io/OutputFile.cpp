// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "tools/io/OutputFile.h"
#include "tools/io/IOException.h"
#include "tools/logger/Logger.h"

#include <cerrno>
#include <fstream>
#include <system_error>

namespace openzl::tools::io {

using namespace openzl::tools::logger;

OutputFile::OutputFile(std::string filename) : filename_(std::move(filename)) {}

poly::string_view OutputFile::name() const
{
    return filename_;
}

void OutputFile::open()
{
    os_.emplace(filename_, std::ios::out | std::ios::binary);
    if (!os_->is_open() || os_->fail()) {
        os_.reset();
        throw IOException("Failed to open output file '" + filename_ + "'");
    }
}

void OutputFile::close()
{
    if (os_) {
        if (os_->is_open()) {
            os_->flush();
            os_->close();
        }
        os_.reset();
    }
}

void OutputFile::write(poly::string_view contents)
{
    if (!os_ || !os_->is_open()) {
        open();
    }
    os_->write(contents.data(), contents.size());
    os_->flush();
    if (os_->fail()) {
        throw IOException("Failed to write to output file '" + filename_ + "'");
    }
}

std::ostream& OutputFile::get_ostream()
{
    if (!os_ || !os_->is_open()) {
        open();
    }
    return os_.value();
}

} // namespace openzl::tools::io
