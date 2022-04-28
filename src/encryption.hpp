#pragma once

#include "cryptopp/cryptlib.h"
#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"

#include "os.hpp"

namespace Encryption {
    // CryptoPP has a TERRIBLE interface
    // this namespace is just for shortcuts to CryptoPP
    namespace CryptoCockAndBalls {
        inline std::string sha256(std::string data)
        {
            char rawDigest[CryptoPP::SHA256::DIGESTSIZE];

            CryptoPP::SHA256().CalculateDigest(
                (CryptoPP::byte*)rawDigest,
                (CryptoPP::byte*)data.data(),
                data.size()
            );

            return std::string(rawDigest, CryptoPP::SHA256::DIGESTSIZE);
        }

        namespace AES {
            inline std::string encrypt(std::string data, std::string key)
            {
                char IV[CryptoPP::AES::BLOCKSIZE];
                memset(IV, 0, CryptoPP::AES::BLOCKSIZE);

                CryptoPP::AES::Encryption aesEncryption((CryptoPP::byte*)key.data(), key.size());
                CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, (CryptoPP::byte*)IV);

                std::string cipher;
                cipher.reserve(data.size() * 2); // approximation

                CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipher));
                stfEncryptor.Put((CryptoPP::byte*)data.data(), data.size());
                stfEncryptor.MessageEnd();

                return cipher;
            }

            inline std::string decrypt(std::string cipher, std::string key)
            {
                char IV[CryptoPP::AES::BLOCKSIZE];
                memset(IV, 0, CryptoPP::AES::BLOCKSIZE);

                CryptoPP::AES::Decryption aesDecryption((CryptoPP::byte*)key.data(), key.size());
                CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, (CryptoPP::byte*)IV);

                std::string data;
                data.reserve(cipher.size()); // approximation

                CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(data));
                stfDecryptor.Put((CryptoPP::byte*)cipher.data(), cipher.size());
                stfDecryptor.MessageEnd();

                return data;
            }
        }
    }

    // some utils, separated to avoid cluttering the main namespace
    namespace Util {
        constexpr inline char BIN_TO_HEX_DIGIT(const char& x)
        {
            constexpr char HEX_DIGITS[] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'A', 'B',
                'C', 'D', 'E', 'F',
            };
            return HEX_DIGITS[x];
        }

        constexpr inline char HEX_DIGIT_TO_BIN(const char& x)
        {
            switch (x) {
            default:
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'A': case 'a': return 10;
            case 'B': case 'b': return 11;
            case 'C': case 'c': return 12;
            case 'D': case 'd': return 13;
            case 'E': case 'e': return 14;
            case 'F': case 'f': return 15;
            }
        }
    }

    namespace Encoding {
        namespace Base64 {
            constexpr char digits[] = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                '4', '5', '6', '7', '8', '9', '+', '/',
            };

            inline std::string encode(const std::string& data)
            {
                const size_t inputSize = data.length();
                const size_t triplets = inputSize / 3;
                const size_t remainingCharacters = inputSize % 3;

                std::string output;
                output.reserve(triplets * 4 + 4);

                const char* ptr = data.data();
                const char* lastTriplet = ptr + 3 * triplets;
                while (ptr <= lastTriplet)
                {
                    const char& a = *ptr++, & b = *ptr++, & c = *ptr++;
                    const char& A = a >> 2, B = (a & 0b11) | (b >> 4), C = (b & 0b1111) | (c >> 6), D = c & 0b111111;

                    output += digits[A];
                    output += digits[B];
                    output += digits[C];
                    output += digits[D];
                }

                switch (remainingCharacters)
                {
                default:
                case 0:
                    break;
                case 1:
                {
                    const char& a = *ptr++;
                    const char& A = a >> 2, B = a & 0b11;
                    output += digits[A];
                    output += digits[B];
                    output += "==";
                    break;
                }
                case 2:
                {
                    const char& a = *ptr++, & b = *ptr++;
                    const char& A = a >> 2, B = (a & 0b11) | (b >> 4), C = (b & 0b1111);
                    output += digits[A];
                    output += digits[B];
                    output += digits[C];
                    output += '=';
                }
                }

                return output;
            }
        
            // TODO:
            //inline std::string decode(const std::string& data)
        }

        namespace Base16 {
            inline std::string encode(const std::string& data)
            {
                std::string output;
                output.reserve(data.size() * 2);

                for (const char& byte : data)
                {
                    output += Util::BIN_TO_HEX_DIGIT(byte >> 4);
                    output += Util::BIN_TO_HEX_DIGIT(byte & 0b1111);
                }

                return output;
            }

            inline std::string decode(const std::string& data)
            {
                std::string output;
                output.reserve(data.size() / 2);

                for (size_t i = 0; i + 1 < data.size(); i += 2)
                {
                    const char& a = data.at(i), b = data.at(i + 1);
                    const char byte = (Util::HEX_DIGIT_TO_BIN(a) << 4) | Util::HEX_DIGIT_TO_BIN(b);
                    output += byte;
                }

                return output;
            }
        }
    }

    // returns a hardware id that should be universally unique, and be constant across multiple calls in different sessions
    inline std::string getHardwareUUID()
    {
        std::string output = OS::system("wmic csproduct get UUID");
        output.erase(std::remove_if(output.begin(), output.end(), [](char c) { return !std::isalnum(c); }), output.end());
        return output;
    }

    // returns the binary digest of the SHA-256 hash of `msg` (will always be 32 characters)
    inline std::string sha256(std::string msg)
    {
        return CryptoCockAndBalls::sha256(msg);
    }

    // gets a 256-bit AES encryption key that is based on hardware and should be constant across sessions
    inline std::string getEncryptionKey()
    {
        return sha256(getHardwareUUID());
    }

    // AES encrypt
    inline std::string encrypt(std::string data, std::string key = "")
    {
        if (key == "")
            key = getEncryptionKey();
        else
            key = sha256(key); // coerce to 256 bits
        
        return CryptoCockAndBalls::AES::encrypt(data, key);
    }

    // AES decrypt
    inline std::string decrypt(std::string data, std::string key = "")
    {
        if (key == "")
            key = getEncryptionKey();
        else
            key = sha256(key); // coerce to 256 bits

        return CryptoCockAndBalls::AES::decrypt(data, key);
    }
}
