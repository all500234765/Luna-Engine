#include "pc.h"
#include "FileSystem.h"

bool FileSystem::FileAccess::open(uint32_t flags, FileNameString fname) {
    mFlags = flags;

    uint32_t access = 0;
    bool ReadOnly = ((flags & Access::Read) == Access::Read);
    access |= (flags & Access::Read ) ? GENERIC_READ    : 0;
    access |= (flags & Access::Write) ? GENERIC_WRITE   : 0;
    access |= (flags & Access::Exec ) ? GENERIC_EXECUTE : 0;

    if( flags & Access::All ) access = GENERIC_ALL;
    
    // Create file handle
    mFile = CreateFile(fname, access, ReadOnly ? FILE_SHARE_READ : FILE_SHARE_WRITE,
                       NULL, (flags & Access::Read) ? OPEN_ALWAYS : CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);

    if( mFile == INVALID_HANDLE_VALUE ) {
        mILED = IEID::CFHF;
        return false;
    }

    // Get file size
    mFileSize = size();

    // Map file
    if( (flags & External::Map) && !map() ) {
        // Try to recreate file handle without file mapping
        CloseHandle(mFile);
        mFile = CreateFile(fname, access, 0, NULL, ReadOnly ? OPEN_ALWAYS : CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);

        if( mFile == INVALID_HANDLE_VALUE ) {
            //mILED = IEID::CMTF; // Old
            mILED = IEID::CFHF2;
            return false;
        }
    }

    seek(Begin, 0);
    seekdir(Right);

    mILED = IEID::Success;
    return true;
}

bool FileSystem::FileAccess::close() {
    if( is_mapped() ) {
        if( !UnmapViewOfFile(mBuffer) ) {
            mILED = IEID::F2UMFV;
            return false;
        }

        if( !CloseHandle(mFileMapping) ) {
            mILED = IEID::F2RFMO;
            return false;
        }
    }

    if( !CloseHandle(mFile) ) {
        mILED = IEID::F2RF;
        return false;
    }

    mILED = IEID::Success;
    return true;
}

bool FileSystem::FileAccess::opened() const {
    return (mFile != INVALID_HANDLE_VALUE);
}

size_t FileSystem::FileAccess::size() const {
    return GetFileSize(mFile, NULL);
}

bool FileSystem::FileAccess::seek(Seek A, size_t n) {
    mCurrentPos = SetFilePointer(mFile, n, 0, A);

    if( mCurrentPos == 0xFFFFFFFF ) {
        mILED = IEID::F2SFP;
        return false;
    }

    mILED = IEID::Success;
    return true;
}

bool FileSystem::FileAccess::seek(size_t n) {
    return seek(mSeek, n);
}

void FileSystem::FileAccess::seek(Seek A) {
    mSeek = A;
}

bool FileSystem::FileAccess::advance(size_t n) {
    return seek(Seek::Relative, n);
}

void FileSystem::FileAccess::seekdir(SeekDir A) {
    mSeekDir = A;
}

bool FileSystem::FileAccess::map() {
    mILED = IEID::Success;

    bool RW1 = ((mFlags & Access::RW) == Access::RW);
    if( !RW1 && (mFlags & Access::Write) ) {
        mILED = IEID::CMFWWOF;
        return false;
    }

    // Create file mapping
    mFileMapping = CreateFileMapping(mFile, NULL, RW1 ? PAGE_READWRITE : PAGE_READONLY, 0, 0, TEXT("MappedName01"));
    if( mFileMapping == NULL || mFileMapping == INVALID_HANDLE_VALUE ) {
        mILED = IEID::CCFMO;
        return false;
    }

    // 
    mBuffer = (iterator)MapViewOfFile(mFileMapping, RW1 ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);

    mCurrentPos = 0;
    mMapped = (mBuffer != 0);
    return mMapped;
}

bool FileSystem::FileAccess::read(char* dest, size_t len) {
    mILED = IEID::Success;

    mRed += len;
    if( is_mapped() ) {
        memcpy_s(&dest[0], len, &mBuffer[mCurrentPos], len);
        mCurrentPos += len;
        return true;
    }

    bool q = ReadFile(mFile, &dest[0], len, &mRed, 0);
    return q;
}

char FileSystem::FileAccess::getch() {
    mRed++;

    // TODO: Check mBuffer[pos]
    if( is_mapped() ) { return mBuffer[mCurrentPos++]; }

    char dest;
    ReadFile(mFile, &dest, 1, &mRed, 0);
    return dest;
}

size_t FileSystem::FileAccess::gets(char* dest) {
    if( is_mapped() ) {
        size_t i = mCurrentPos, beg = i;
        while( i < mFileSize && mBuffer[i] != '\3' && mBuffer[i] != '\0' ) { i++; }
        i -= (mBuffer[i] != '\0'); // It might be eof

        read(dest, i - beg); // TODO: Check
        mCurrentPos += 2; // Skip \0 and \n
        return i - beg; // TODO: Check
    }

    size_t beg = mCurrentPos, i = beg;
    char ch; read(&ch, 1);

    while( ((i - beg) < 1024) && (i < mFileSize) && ch != '\3' && ch != '\0' ) {
        dest[i++ - beg] = ch; // TODO: Check
        read(&ch, 1);
    }

    advance((size_t)((ch == '\0') || (ch == '\r') || (ch == '\n')));

    return i - beg; // TODO: Check
}

bool FileSystem::FileAccess::cread(char* dest, size_t len) const {
    if( is_mapped() ) {
        memcpy_s(&dest[0], len, &mBuffer[mCurrentPos], len);
        return true;
    }

    DWORD ind = mRed + len;
    bool q = ReadFile(mFile, &dest[0], len, &ind, 0);
    return q;
}

char FileSystem::FileAccess::cgetch() const {
    // TODO: Check mBuffer[pos]
    if( is_mapped() ) { return mBuffer[mCurrentPos]; }

    char dest;
    DWORD ind = mRed + 1;
    ReadFile(mFile, &dest, 1, &ind, 0);
    return dest;
}

char* FileSystem::FileAccess::getline(size_t& len) {
    char dest[1024]{};

    if( is_mapped() ) {
        size_t i = mCurrentPos, beg = i;
        while( i < mFileSize && mBuffer[i] != '\3' && mBuffer[i] != '\n' ) { i++; }
        i -= (mBuffer[i] != '\0'); // It might be eof

        read(dest, i - beg + 1);
        mCurrentPos += 1; // Skip \0 and \n
        len = i - beg + 1;
        return dest;
    }

    size_t beg = mCurrentPos, i = beg;
    char ch; read(&ch, 1);

    while( ((i - beg) < 1024) && (i < mFileSize) && ch != '\3' && ch != '\n' && ch != '\r' ) {
        dest[i ++- beg] = ch;
        read(&ch, 1);
    }

    advance((size_t)((ch == '\0') || (ch == '\r') || (ch == '\n')));

    len = i - beg; // TODO: Check
    return dest; // TODO: Check
}

size_t FileSystem::FileAccess::operator>>(char* dest) {
    size_t len;
    dest = getline(len);
    return len;
}

bool FileSystem::FileAccess::operator>>(char dest) {
    return read(&dest, 1);
}

bool FileSystem::FileAccess::berror() const {
    return mILED != Success;
}

DWORD FileSystem::FileAccess::cerror() const {
    return GetLastError();
}

FileSystem::IEID FileSystem::FileAccess::nerror() const {
    return mILED;
}

bool FileSystem::FileAccess::can_read() const {
    return mFlags & Access::Read;
}

bool FileSystem::FileAccess::can_write() const {
    return mFlags & Access::Write;
}

bool FileSystem::FileAccess::is_mapped() const {
    return (mFlags & External::Map) && (mFileMapping != INVALID_HANDLE_VALUE) && mMapped;
}

FileSystem::SeekDir FileSystem::FileAccess::get_seekdir() const {
    return mSeekDir;
}

FileSystem::Seek FileSystem::FileAccess::get_seek() const {
    return mSeek;
}

bool FileSystem::FileAccess::flush() {
    if( !can_write() ) return false;

    mILED = IEID::Success;
    if( is_mapped() ) {
        if( !FlushViewOfFile(mBuffer, 0) ) {
            mILED = IEID::FFFV;
            return false;
        }

        return true;
    }

    FlushFileBuffers(mFile);
    mILED = IEID::TODO;
    return true;
}

bool FileSystem::FileAccess::write(char* src, size_t len) {
    if( is_mapped() ) {
        // TODO: Resize buffer & file 
        // Add check for buffer overflow to impl everything before this line
        return false;
        memcpy(&mBuffer[mCurrentPos], src, len);
        mCurrentPos += len;
        return true;
    }

    return WriteFile(mFile, (LPCVOID)&src[0], len, &mWritten, 0);
}

bool FileSystem::FileAccess::write(LPCVOID src, size_t len) {
    if( is_mapped() ) {
        // TODO: Resize buffer & file 
        // Add check for buffer overflow to impl everything before this line
        return false;
        memcpy(&mBuffer[mCurrentPos], src, len);
        mCurrentPos += len;
        return true;
    }

    return WriteFile(mFile, src, len, &mWritten, 0);
}

bool FileSystem::FileAccess::putch(char src) {
    if( is_mapped() ) {
        // TODO: Resize buffer & file 
        // Add check for buffer overflow to impl everything before this line
        return false;
        memcpy(&mBuffer[mCurrentPos], &src, 1);
        mCurrentPos++;
        return true;
    }

    return WriteFile(mFile, (LPCVOID)&src, 1, &mWritten, 0);
}

bool FileSystem::FileAccess::eol() {
    return putch('\n');
}

bool FileSystem::FileAccess::eof() {
    mILED = IEID::Success;

    if( is_mapped() ) {
        mILED = IEID::CAEOF2MF;
        return false;
    }

    if( !SetEndOfFile(mFile) ) {
        mILED = IEID::F2SEOF;
        return false;
    }

    return true;
}

bool FileSystem::FileAccess::operator<<(char* src) {
    return write(src, strlen(src));
}

bool FileSystem::FileAccess::operator<<(char src) {
    return write(&src, 1);
}

bool FileSystem::FileAccess::is_eol() const {
    if( is_mapped() ) {
        return mBuffer[mCurrentPos] == '\n';
    }

    char ch;
    cread(&ch, 1);
    return ch == '\n';
}

bool FileSystem::FileAccess::is_eof() const {
    return mCurrentPos >= mFileSize - 1;
}

char FileSystem::FileAccess::operator[](size_t n) {
    if( is_mapped() ) {
        return mBuffer[n];
    }

    char ch;
    read(&ch, 1);
    return ch;
}
