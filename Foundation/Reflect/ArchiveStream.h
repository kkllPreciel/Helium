#pragma once

#include "API.h"  
#include "Exceptions.h"

#include "Platform/Assert.h"

#include "Foundation/File/Path.h"
#include "Foundation/Memory/Endian.h"

#ifdef UNICODE

// http://www.codeproject.com/KB/stl/upgradingstlappstounicode.aspx

class null_codecvt : public std::codecvt< wchar_t , char , mbstate_t >
{
public:
    typedef wchar_t _E;
    typedef char _To;
    typedef mbstate_t _St;

    explicit null_codecvt( size_t _R=0 ) : std::codecvt< wchar_t , char , mbstate_t >(_R)
    {

    }

    static null_codecvt& GetStaticInstance()
    {
        return sm_StaticInstance;
    }

protected:
    virtual result do_in( _St& _State , const _To* _F1 , const _To* _L1 , const _To*& _Mid1 , _E* F2 , _E* _L2 , _E*& _Mid2 ) const HELIUM_OVERRIDE
    {
        return noconv;
    }
    virtual result do_out( _St& _State , const _E* _F1 , const _E* _L1 , const _E*& _Mid1 , _To* F2, _To* _L2 , _To*& _Mid2 ) const HELIUM_OVERRIDE
    {
        return noconv;
    }
    virtual result do_unshift( _St& _State , _To* _F2 , _To* _L2 , _To*& _Mid2 ) const HELIUM_OVERRIDE
    {
        return noconv;
    }
    virtual int do_length( const _St& _State , const _To* _F1 , const _To* _L1 , size_t _N2 ) const throw () HELIUM_OVERRIDE
    {
        return (int)( (_N2 < (size_t)(_L1 - _F1)) ? _N2 : _L1 - _F1 );
    }
    virtual bool do_always_noconv() const throw () HELIUM_OVERRIDE
    {
        return true;
    }
    virtual int do_max_length() const throw () HELIUM_OVERRIDE
    {
        return 2;
    }
    virtual int do_encoding() const throw () HELIUM_OVERRIDE
    {
        return 2;
    }

private:
    static null_codecvt sm_StaticInstance;
};

#endif

namespace Helium
{
    namespace Reflect
    {
        extern Profile::Accumulator g_StreamWrite;
        extern Profile::Accumulator g_StreamRead; 

        //
        // How characters are encoded
        //

        namespace CharacterEncodings
        {
            enum CharacterEncoding
            {
                ASCII,  // default encoding, legacy 7-bit
                UTF_16, // used by windows' Unicode build
            };
        }
        typedef CharacterEncodings::CharacterEncoding CharacterEncoding;

        //
        // Stream object, read and write data to/from a buffer
        //

        template< class StreamCharT >
        class Stream : public Helium::RefCountBase< Stream< StreamCharT > >
        {
        public: 
            Stream( ByteOrder byteOrder = ByteOrders::LittleEndian, CharacterEncoding characterEncoding = CharacterEncodings::ASCII )
                : m_Stream( NULL )
                , m_ByteOrder( byteOrder )
                , m_CharacterEncoding( characterEncoding )
                , m_OwnStream( false )
            {

            }

            Stream( std::basic_iostream< StreamCharT, std::char_traits< StreamCharT > >* stream, bool ownStream, ByteOrder byteOrder = ByteOrders::LittleEndian, CharacterEncoding characterEncoding = CharacterEncodings::ASCII )
                : m_Stream( stream )
                , m_OwnStream( ownStream )
                , m_ByteOrder( byteOrder )
                , m_CharacterEncoding( characterEncoding )
            {

            }

            virtual ~Stream()
            {
                if ( m_OwnStream )
                {
                    delete m_Stream; 
                    m_Stream    = NULL; 
                    m_OwnStream = false; 
                }
            }

            virtual void Open()
            {

            }

            virtual void Close()
            {

            }

            void Clear()
            {
                m_Stream->clear(); 
            }

            void SetPrecision(int p)
            {
                m_Stream->precision(p); 
            }

            void SkipWhitespace()
            {
                *m_Stream >> std::ws; 
            }

            std::streampos TellRead()
            {
                return m_Stream->tellg(); 
            }

            std::streampos TellWrite()
            {
                return m_Stream->tellp(); 
            }

            std::streamsize ElementsRead()
            {
                return m_Stream->gcount(); 
            }

            std::streamsize ElementsAvailable()
            {
                return m_Stream->rdbuf()->in_avail();
            }

            Stream& SeekRead(std::streamoff offset, std::ios_base::seekdir dir)
            {
                m_Stream->seekg(offset, dir); 
                return *this; 
            }

            Stream& SeekWrite(std::streamoff offset, std::ios_base::seekdir dir)
            {
                m_Stream->seekp(offset, dir); 
                return *this; 
            }

            Stream& ReadBuffer(void* t, std::streamsize streamElementCount)
            {
                PROFILE_SCOPE_ACCUM(g_StreamRead); 

                m_Stream->read((StreamCharT*)t, streamElementCount); 

                if (m_Stream->fail() && !m_Stream->eof())
                {
                    throw Reflect::StreamException( TXT( "General read failure" ) ); 
                }

                return *this; 
            }

            template <typename PointerT>
            inline Stream& Read(PointerT* ptr)
            {
                // amount to read must align with stream element size
                HELIUM_COMPILE_ASSERT( sizeof(PointerT) % sizeof(StreamCharT) == 0  );
                return ReadBuffer( (StreamCharT*)ptr, sizeof(PointerT) / sizeof(StreamCharT) );
                Swizzle( ptr, m_ByteOrder != Helium::PlatformByteOrder );
            }

            Stream& WriteBuffer(const void* t, std::streamsize streamElementCount)
            {
                PROFILE_SCOPE_ACCUM(g_StreamWrite); 

                m_Stream->write( (const StreamCharT*)t, streamElementCount );

                if (m_Stream->fail())
                {
                    throw Reflect::StreamException( TXT( "General write failure") ); 
                }

                return *this;
            }

            template <typename PointerT>
            inline Stream& Write(const PointerT* ptr)
            {
                // amount to write must align with stream element size
                HELIUM_COMPILE_ASSERT( sizeof(PointerT) % sizeof(StreamCharT) == 0  );
                PointerT temp = *ptr;
                Swizzle( temp, m_ByteOrder != Helium::PlatformByteOrder );
                return WriteBuffer( (const StreamCharT*)&temp, sizeof(PointerT) / sizeof(StreamCharT) ); 
            }

            template <typename PointerT>
            inline Stream& ReadArray( PointerT* elements, size_t count )
            {
                for ( int i = 0; i < count; ++i )
                {
                    Read( elements[ i ] );
                }

                return *this;
            }

            template <typename PointerT>
            inline Stream& WriteArray( const PointerT* elements, size_t count )
            {
                for ( int i = 0; i < count; ++i )
                {
                    Write( elements[ i ] );
                }

                return *this;
            }

            inline Stream& ReadString( tstring& string )
            {
                uint32_t length = 0;
                Read( &length );

                switch ( m_CharacterEncoding )
                {
                case CharacterEncodings::ASCII:
                    {
#ifdef UNICODE
                        std::string temp;
                        temp.resize( length );
                        ReadBuffer( &temp[ 0 ], length );
                        Helium::ConvertString( temp, string );
#else
                        // read the bytes directly into the string
                        string.resize( length ); 
                        ReadBuffer( &string[ 0 ], length ); 
#endif
                        break;
                    }

                case CharacterEncodings::UTF_16:
                    {
#ifdef UNICODE
                        // read the bytes directly into the string
                        string.resize( length ); 
                        ReadBuffer( &string[ 0 ], length * 2 ); 
#else
                        std::wstring temp;
                        temp.resize( length );
                        ReadBuffer( &temp[ 0 ], length * 2 ); 
                        Helium::ConvertString( temp, string );
#endif
                        break;
                    }
                }
                return *this;
            }

            inline Stream& WriteString( const tstring& string )
            {
                uint32_t length = (uint32_t)string.length();
                Write( &length );
                WriteBuffer( string.c_str(), length * sizeof(tchar_t) );
                return *this;
            }

            Stream& Flush()
            {
                m_Stream->flush(); 
                return *this; 
            }

            bool Fail()
            {
                return m_Stream->fail(); 
            }

            bool Done()
            {
                return m_Stream->eof(); 
            }

            std::basic_iostream< StreamCharT, std::char_traits< StreamCharT > >& GetInternal()
            {
                return *m_Stream;
            } 

            const ByteOrder& GetByteOrder()
            {
                return m_ByteOrder;
            }

            const CharacterEncoding& GetCharacterEncoding()
            {
                return m_CharacterEncoding;
            }

        protected: 
            std::basic_iostream< StreamCharT, std::char_traits< StreamCharT > >*    m_Stream; 
            ByteOrder                                                               m_ByteOrder;
            CharacterEncoding                                                       m_CharacterEncoding;
            bool                                                                    m_OwnStream;
        };

        template <class T, class StreamCharT>
        Stream< StreamCharT >& operator>>(Stream< StreamCharT >& stream, T& val)
        {
            HELIUM_ASSERT( stream.GetByteOrder() == Helium::PlatformByteOrder );
            stream.GetInternal() >> val;

            if(stream.Fail() && !stream.Done())
            {
                throw Reflect::StreamException( TXT( "General read failure" ) ); 
            }

            return stream; 
        }

        template <class T, class StreamCharT>
        Stream< StreamCharT >& operator<<(Stream< StreamCharT >& stream, const T& val)
        {
            HELIUM_ASSERT( stream.GetByteOrder() == Helium::PlatformByteOrder );
            stream.GetInternal() << val;

            if(stream.Fail())
            {
                throw Reflect::StreamException( TXT( "General write failure" ) ); 
            }

            return stream; 
        }

        //
        // Specializations
        //

        template <>
        inline Stream<wchar_t>& operator>>(Stream<wchar_t>& stream, uint8_t& val)
        {
            uint16_t temp;
            stream.GetInternal() >> temp;

            if(stream.Fail() && !stream.Done())
            {
                throw Reflect::StreamException( TXT( "General read failure" ) ); 
            }

            val = (uint8_t)temp;

            return stream; 
        }

        template <>
        inline Stream<wchar_t>& operator<<(Stream<wchar_t>& stream, const uint8_t& val)
        {
            uint16_t temp = val;
            stream.GetInternal() << temp; 

            if(stream.Fail())
            {
                throw Reflect::StreamException( TXT( "General write failure" ) ); 
            }

            return stream; 
        }

        template <>
        inline Stream<char>& operator>>(Stream<char>& stream, uint8_t& val)
        {
            uint16_t temp;
            stream.GetInternal() >> temp;

            if(stream.Fail() && !stream.Done())
            {
                throw Reflect::StreamException( TXT( "General read failure" ) ); 
            }

            val = (uint8_t)temp;

            return stream; 
        }

        template <>
        inline Stream<char>& operator<<(Stream<char>& stream, const uint8_t& val)
        {
            uint16_t temp = val;
            stream.GetInternal() << temp; 

            if(stream.Fail())
            {
                throw Reflect::StreamException( TXT( "General write failure" ) ); 
            }

            return stream; 
        }

        template <>
        inline Stream<wchar_t>& operator>>(Stream<wchar_t>& stream, int8_t& val)
        {
            uint16_t temp;
            stream.GetInternal() >> temp;

            if(stream.Fail() && !stream.Done())
            {
                throw Reflect::StreamException( TXT( "General read failure" ) ); 
            }

            val = (int8_t)temp;

            return stream; 
        }

        template <>
        inline Stream<wchar_t>& operator<<(Stream<wchar_t>& stream, const int8_t& val)
        {
            uint16_t temp = val;
            stream.GetInternal() << temp; 

            if(stream.Fail())
            {
                throw Reflect::StreamException( TXT( "General write failure" ) ); 
            }

            return stream; 
        }

        template <>
        inline Stream<char>& operator>>(Stream<char>& stream, int8_t& val)
        {
            uint16_t temp;
            stream.GetInternal() >> temp;

            if(stream.Fail() && !stream.Done())
            {
                throw Reflect::StreamException( TXT( "General read failure" ) ); 
            }

            val = (int8_t)temp;

            return stream; 
        }

        template <>
        inline Stream<char>& operator<<(Stream<char>& stream, const int8_t& val)
        {
            uint16_t temp = val;
            stream.GetInternal() << temp; 

            if(stream.Fail())
            {
                throw Reflect::StreamException( TXT( "General write failure" ) ); 
            }

            return stream; 
        }

        typedef Stream<char> CharStream;
        typedef Stream<wchar_t> WCharStream;
        typedef Stream<tchar_t> TCharStream;

        typedef Helium::SmartPtr< Stream<char> > CharStreamPtr; 
        typedef Helium::SmartPtr< Stream<wchar_t> > WCharStreamPtr; 
        typedef Helium::SmartPtr< Stream<tchar_t> > TCharStreamPtr;

        //
        // FileStream, a stream object backed by file data
        //

        template< class StreamCharT >
        class FileStream : public Stream< StreamCharT >
        {
        public: 
            FileStream( const Path& path, bool write, const ByteOrder byteOrder = Helium::PlatformByteOrder )
                : m_Path( path )
                , m_OpenForWrite( write )
            {
                m_ByteOrder = byteOrder;
            }

            ~FileStream()
            {

            }

            virtual void Open() HELIUM_OVERRIDE
            {
                // deal with the mode bits.. 
                int fmode = std::ios_base::binary;
                if (m_OpenForWrite)
                {
                    fmode |= std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
                    m_Path.MakePath();
                }
                else
                {
                    fmode |= std::ios_base::in;
                }

                std::basic_fstream< StreamCharT, std::char_traits< StreamCharT > >* fstream = new std::basic_fstream< StreamCharT, std::char_traits< StreamCharT > >(); 

#ifdef UNICODE
                fstream->imbue( std::locale( std::locale::classic(), &null_codecvt::GetStaticInstance() )) ;
#endif

                fstream->open( m_Path.c_str(), fmode );
                if (!fstream->is_open())
                {
                    delete fstream;
                    throw Reflect::StreamException( TXT( "Unable to open '%s' for %s" ) , m_Path.c_str(), m_OpenForWrite ? TXT( "write" ) : TXT( "read" ));
                }

                m_Stream    = fstream; 
                m_OwnStream = true; 
            }

            virtual void Close() HELIUM_OVERRIDE
            {
                std::basic_fstream< StreamCharT, std::char_traits< StreamCharT > >* fstream = static_cast< std::basic_fstream< StreamCharT, std::char_traits< StreamCharT > > *>( &GetInternal() );

                fstream->close();
                if (fstream->is_open())
                {
                    throw Reflect::StreamException( TXT( "Unable to close '%s' after %s" ), m_Path.c_str(), m_OpenForWrite ? TXT( "write" ) : TXT( "read" ));
                }
            }

        protected: 
            Path  m_Path; 
            bool  m_OpenForWrite; 
        };

        typedef FileStream<char> CharFileStream;
        typedef FileStream<wchar_t> WCharFileStream;
        typedef FileStream<tchar_t> TCharFileStream;

        typedef Helium::SmartPtr< FileStream<char> > CharFileStreamPtr; 
        typedef Helium::SmartPtr< FileStream<wchar_t> > WCharFileStreamPtr; 
        typedef Helium::SmartPtr< FileStream<tchar_t> > TCharFileStreamPtr;
    }
}