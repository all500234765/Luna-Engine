#pragma once

#include "pc.h"

namespace FileSystem {

    enum Access {
        Read = 1, Write = 2,
        RW = Read | Write, Exec = 4,
        All = 8, 

        AReserved = 1 << 4,
        AMask = AReserved - 1
    };

    enum Seek { // Default: Begin
        Begin, Relative, End
    };

    enum SeekDir { // Default: Right
        Right, Left
    };

    // TODO: Security update
    enum Security {


        SReserved = Access::AReserved << 8,
        SMask = (SReserved - 1) & ~(Access::AMask)
    };

    enum External {
        Map = Security::SReserved,


        EReserved = Security::SReserved << 4,
        EMask = (EReserved - 1) & ~(Security::SMask)
    };

    enum IEID {
        Success = 0, 

        // Open
        CMTF = 1, // Cannot map the file
        CFHF = 2, // Creation of file handle failed
        CFHF2 = 12,  // Cannot create file handle after mapping fail

        // Map
        CCFMO = 3,   // Cannot create file mapping object
        CMFWWOF = 4, // Cannot map file with write only flag

        // Close
        F2UMFV = 5, // Failed to unmap file view
        F2RFMO = 6, // Failed to release file mapping object
        F2RF   = 7, // Failed to release file

        // Flush
        FFFV = 8, // Failed to flush file view

        // Eof
        CAEOF2MF = 9, // Cannot add eof 2 mapped file
        F2SEOF = 10, // Failed to set eof

        // Seek
        F2SFP = 11, // Failed to set file pointer


        // Misc
        TODO = 126, 
        NotSupportedYet = 127, 
    };

#ifdef UNICODE
#define FileNameString const wchar_t*
#else
#define FileNameString const char*
#endif // !UNICODE

    // 
    class FileAccess {
    public:
        // Iterator
        typedef char* iterator;
        typedef const char* const_iterator;

        iterator begin()             { return &mBuffer[0]; }
        const_iterator begin() const { return &mBuffer[0]; }
        iterator end()               { return &mBuffer[mFileSize]; }
        const_iterator end()   const { return &mBuffer[mFileSize]; }

    protected:
        uint32_t mFlags{};
        size_t mFileSize{};
        
        HANDLE mFile{}, mFileMapping{}, mView{};
        iterator mBuffer{};
        DWORD mCurrentPos{}, mRed{}, mWritten{};

        Seek mSeek{};
        SeekDir mSeekDir{};
        IEID mILED{}; // InternalLastErrorID

        bool mMapped{};

        // 
        bool open(uint32_t flags, FileNameString fname);
        bool close();

    public:
        // Generic
        bool opened() const; // Is file opened
        size_t size() const; // Get file size

        bool seek(Seek A, size_t n); // Move caret by n bytes relative to A (func doesn't save A as new seek state)
        bool seek(size_t n); // Seek n bytes based on current seek params
        void seek(Seek A); // Set seek type
        bool advance(size_t n);  // Move caret by n bytes in seekdir
        void seekdir(SeekDir A); // Set seek direction

        bool berror() const; // Check for last error
        DWORD cerror() const; // Returns last error
        char* errstr() const; // Returns human readable string for error
        IEID nerror() const; // Returns current error IEID

        bool can_read() const; // flags & Access::Read == Access::Read
        bool can_write() const; // flags & Access::Write == Access::Write
        bool is_mapped() const; // flags & External::Map == External::Map

        SeekDir get_seekdir() const;  // Get current seek direction
        Seek get_seek() const; // Get current seek type

        size_t tell() const; // Get current caret position
        size_t written() const; // Get amount of bytes written
        size_t red() const; // Get amount of bytes red

        bool map(); // Map file if it hasn't been mapped

        // Access::Read
        bool read(char* dest, size_t len); // Read data
        char getch(); // Read char
        size_t gets(char* dest); // Read char*

        char* getline(size_t& len); // Read whole line

        template<typename T>
        bool   operator>>(T     dest); // Read any type
        size_t operator>>(char* dest); // Read string
        bool   operator>>(char  dest); // Read character

        bool is_eol() const; // Is current byte eol (doesn't advance curret position)
        bool is_eof() const; // Is current byte eof (doesn't advance curret position)

        char operator[](size_t n); // Read nth byte

        // Access::Write
        bool flush(size_t len) const; // (Mapped only) Flush n bytes from current position
        bool flush(Seek A, size_t len) const; // (Mapped only) Flush n bytes from A
        bool flush(size_t beg, size_t len) const; // (Mapped only) Flush n bytes from beg

        bool fill(char A, size_t len); // Write len bytes of A
        bool flush(); // Flush contents of current file
        bool write(char* src, size_t len); // Write data
        bool write(LPCVOID src, size_t len); // Write data
        bool putch(char src); // Write char
        bool puts(char* src); // Write char*
        bool eol(); // Set eol at current position
        bool eof(); // Set eof at current position

        template<typename T>
        bool operator<<(T     src); // Write any type
        bool operator<<(char* src); // Write string
        bool operator<<(char  src); // Write character

        void operator=(char* src);
        void operator=(char  src);
        //file[0] = value; // Write to 0th byte
    };

    template<typename T>
    bool FileSystem::FileAccess::operator>>(T dest) {
        return read((char*)&dest, sizeof(T));
    }

    template<typename T>
    bool FileSystem::FileAccess::operator<<(T dest) {
        return write((char*)dest, sizeof(T));
    }

    // 
    class AccessFile: public FileAccess {
    public:
        AccessFile(uint32_t flags, FileNameString fname) { open(flags, fname); }

        bool fclose() { return close(); };
    };

    // 
    class Scoped: public FileAccess {
    public:
        Scoped(uint32_t flags, FileNameString fname) { open(flags, fname); }
        ~Scoped() { close(); }
    };

    template<uint32_t flags>
    class ScopedFileAccessFlagsTemplate: public FileAccess {
    public:
        ScopedFileAccessFlagsTemplate(FileNameString fname) { open(flags, fname); }
        ~ScopedFileAccessFlagsTemplate() { close(); }
    };

    class ScopedConfigFileImpl: public FileAccess {
    public:
        ScopedConfigFileImpl(FileNameString fname) { open(Access::RW, fname); }
        ~ScopedConfigFileImpl() { eof(); close(); };

        template<typename T>
        bool write_param(const char* name, T value) {
            bool r = true;
            r &= write((char*)name, strlen(name));
            r &= write((char*)" = ", 3);
            r &= write(&value, sizeof(T));
            r &= eol();

            return r;
        }

        template<>
        bool write_param<const char*>(const char* name, const char* value) {
            bool r = true;
            r &= write((char*)name, strlen(name));
            r &= write((char*)" = ", 3);
            r &= write((char*)&value, strlen(value));
            r &= eol();

            return r;
        }

        template<>
        bool write_param<char*>(const char* name, char* value) {
            bool r = true;
            r &= write((char*)name, strlen(name));
            r &= write((char*)" = ", 3);
            r &= write(value, strlen(value));
            r &= eol();

            return r;
        }

        template<>
        bool write_param<bool>(const char* name, bool value) {
            static const char* bool2str[] = { "false", "true" };

            bool r = true;
            r &= write((char*)name, strlen(name));
            r &= write((char*)" = ", 3);
            r &= write((char*)bool2str[value], 4 + !value);
            r &= eol();

            return r;
        }
    };

};

// Default templates: 
typedef FileSystem::ScopedConfigFileImpl ScopedConfigFile;
typedef FileSystem::Scoped ScopedFileAccess;
typedef FileSystem::AccessFile FileAccess;

typedef FileSystem::ScopedFileAccessFlagsTemplate<FileSystem::Access::Write                           > ScopedFileAccessW;
typedef FileSystem::ScopedFileAccessFlagsTemplate<FileSystem::Access::Read | FileSystem::External::Map> ScopedFileAccessRM;
typedef FileSystem::ScopedFileAccessFlagsTemplate<FileSystem::Access::Read                            > ScopedFileAccessR;
typedef FileSystem::ScopedFileAccessFlagsTemplate<FileSystem::Access::RW | FileSystem::External::Map  > ScopedFileAccessRWM;
typedef FileSystem::ScopedFileAccessFlagsTemplate<FileSystem::Access::RW                              > ScopedFileAccessRW;


