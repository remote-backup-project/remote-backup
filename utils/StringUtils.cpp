#include "StringUtils.h"
#include <search.h>
#include <openssl/md5.h>
#include <iomanip>
#include <sstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "Logger.h"

std::string StringUtils::encodeBase64(const std::string& data) {
    static constexpr char sEncodingTable[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t in_len = data.size();
    if(in_len == 0)
        return "";
    size_t out_len = 4 * ((in_len + 2) / 3);
    std::string ret(out_len, '\0');
    size_t i;
    char *p = const_cast<char*>(ret.c_str());

    for (i = 0; i < in_len - 2; i += 3) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int) (data[i + 2] & 0xC0) >> 6)];
        *p++ = sEncodingTable[data[i + 2] & 0x3F];
    }
    if (i < in_len) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        if (i == (in_len - 1)) {
            *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else {
            *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    return ret;
}

std::string StringUtils::decodeBase64(const std::string& input, std::string& out) {
    static constexpr unsigned char kDecodingTable[] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
            64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
            64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    size_t in_len = input.size();
    if (in_len % 4 != 0) return "Input data size is not a multiple of 4";
    if(in_len == 0) return "";

    size_t out_len = in_len / 4 * 3;
    if (input[in_len - 1] == '=') out_len--;
    if (input[in_len - 2] == '=') out_len--;

    out.resize(out_len);

    for (size_t i = 0, j = 0; i < in_len;) {
        uint32_t a = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];
        uint32_t b = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];
        uint32_t c = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];
        uint32_t d = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];

        uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        if (j < out_len) out[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < out_len) out[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < out_len) out[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return "";
}

std::string StringUtils::getStringDifference(const std::string& s1, const std::string& s2){
    for(long unsigned int i = 0; i < s1.size(); i++){
        if(s2.size() > i && s1[i] == s2[i])
            continue;
        else return s1.substr(i, s1.size()-1);
    }
    return "";
}

std::vector<std::string> StringUtils::split(const std::string& s, const std::string& delimitator){
    std::vector<std::string> results;
    boost::split(results, s, boost::is_any_of(delimitator));
    return results;
}

void StringUtils::fillStreambuf(boost::asio::streambuf& streambuf, std::string string){
    std::ostream request_stream(&streambuf);
    request_stream << string;
}

std::string StringUtils::fillFromStreambuf(boost::asio::streambuf& streambuf){
    std::istream is(&streambuf);
    std::string s;
    is >> s;
    return s;
}

std::string StringUtils::response_to_string(StockResponse::StatusType status){
    switch (status)
    {
        case StockResponse::continue_:
            return StockResponse::CONTINUE;
        case StockResponse::ok:
            return StockResponse::OK;
        case StockResponse::bad_request:
            return StockResponse::BAD_REQUEST;
        case StockResponse::unauthorized:
            return StockResponse::UNAUTHORIZED;
        case StockResponse::forbidden:
            return StockResponse::FORBIDDEN;
        case StockResponse::not_found:
            return StockResponse::NOT_FOUND;
        case StockResponse::internal_server_error:
            return StockResponse::INTERNAL_SERVER_ERROR;
        default:
            return StockResponse::INTERNAL_SERVER_ERROR;
    }
}

std::string StringUtils::md5FromFile(const std::string &path)
{
    unsigned char result[MD5_DIGEST_LENGTH];
    std::ifstream ifs(path, std::ios::in | std::ios::binary);

    MD5_CTX md5;
    MD5_Init(&md5);
    std::vector<char> data(Socket::CHUNK_SIZE + 1, 0);
    while(ifs)
    {
        ifs.read(data.data(), Socket::CHUNK_SIZE);
        std::streamsize s = ifs.gcount();
        data[s] = 0;
        MD5_Update(&md5, data.data(), s);
    }
    ifs.close();
    MD5_Final(result, &md5);

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');

    for(int c : result)
        sout << std::setw(2) << (int)c;

    return sout.str();
}
