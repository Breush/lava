#include <cstdio>
#include <lava/chamber/Err.hpp>
#include <streambuf>

namespace {
    // This class will be used as the default streambuf of lava::Err,
    // it outputs to stderr by default (to keep the default behavior)
    class DefaultErrStreamBuf : public std::streambuf {
    public:
        DefaultErrStreamBuf()
        {
            // Allocate the write buffer
            static const int size = 64;
            char* buffer = new char[size];
            setp(buffer, buffer + size);
        }

        ~DefaultErrStreamBuf()
        {
            // Synchronize
            sync();

            // Delete the write buffer
            delete[] pbase();
        }

    private:
        virtual int overflow(int character)
        {
            if ((character != EOF) && (pptr() != epptr())) {
                // Valid character
                return sputc(static_cast<char>(character));
            }
            else if (character != EOF) {
                // Not enough space in the buffer: synchronize output and try again
                sync();
                return overflow(character);
            }
            else {
                // Invalid character: synchronize output
                return sync();
            }
        }

        virtual int sync()
        {
            // Check if there is something into the write buffer
            if (pbase() != pptr()) {
                // Print the contents of the write buffer into the standard error output
                std::size_t size = static_cast<int>(pptr() - pbase());
                fwrite(pbase(), 1, size, stderr);

                // Reset the pointer position to the beginning of the write buffer
                setp(pbase(), epptr());
            }

            return 0;
        }
    };
}

namespace lava {
    std::ostream& err()
    {
        static DefaultErrStreamBuf buffer;
        static std::ostream stream(&buffer);

        return stream;
    }
}
