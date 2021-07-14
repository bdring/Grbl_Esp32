#include <FS.h>

using namespace fs;

size_t File::write(uint8_t c) {
    return 0;
}

time_t File::getLastWrite() {
    return 0;
}

size_t File::write(const uint8_t* buf, size_t size) {
    return 0;
}

int File::available() {
    return false;
}

int File::read() {
    return -1;
}

size_t File::read(uint8_t* buf, size_t size) {
    return -1;
}

int File::peek() {
    return -1;
}

void File::flush() {}

bool File::seek(uint32_t pos, SeekMode mode) {
    return false;
}

size_t File::position() const {
    return 0;
}

size_t File::size() const {
    return 0;
}

void File::close() {}

File::operator bool() const {
    return false;
}

const char* File::name() const {
    return nullptr;
}

bool File::isDirectory(void) {
    return false;
}

File File::openNextFile(const char* mode) {
    return File();
}

void File::rewindDirectory(void) {}

File FS::open(const String& path, const char* mode) {
    return open(path.c_str(), mode);
}

File FS::open(const char* path, const char* mode) {
    return File();
}

bool FS::exists(const char* path) {
    return false;
}

bool FS::exists(const String& path) {
    return exists(path.c_str());
}

bool FS::remove(const char* path) {
    return false;
}

bool FS::remove(const String& path) {
    return remove(path.c_str());
}

bool FS::rename(const char* pathFrom, const char* pathTo) {
    return false;
}

bool FS::rename(const String& pathFrom, const String& pathTo) {
    return rename(pathFrom.c_str(), pathTo.c_str());
}

bool FS::mkdir(const char* path) {
    return false;
}

bool FS::mkdir(const String& path) {
    return mkdir(path.c_str());
}

bool FS::rmdir(const char* path) {
    return false;
}

bool FS::rmdir(const String& path) {
    return rmdir(path.c_str());
}

#if 0
void FSImpl::mountpoint(const char* mp) {}

const char* FSImpl::mountpoint() {
    return "";
}
#endif
