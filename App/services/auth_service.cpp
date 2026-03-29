#include "auth_service.h"

AuthService::AuthService() {}

void AuthService::setKeyPath(const QString& key_path)
{
    _key_path = key_path;
}

std::uint8_t AuthService::obf_key(std::uint8_t k)
{
    return static_cast<std::uint8_t>(k ^ key_mask);
}

void AuthService::xor_inplace(std::string& s, unsigned char key)
{
    for (auto& ch : s)
        ch = static_cast<char>(static_cast<unsigned char>(ch) ^ key);
}

std::string AuthService::xor_copy(const std::string& in, std::uint8_t key)
{
    std::string out = in;
    xor_inplace(out, key);
    return out;
}

std::string AuthService::bytes_to_hex(const std::string& s)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const auto byte = static_cast<unsigned int>(
            static_cast<unsigned char>(s[i]));
        oss << std::setw(2) << byte;
        if (i + 1 < s.size())
            oss << ' ';
    }
    return oss.str();
}

bool AuthService::read_u32(std::ifstream& in, std::uint32_t& v)
{
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return static_cast<bool>(in);
}

bool AuthService::read_u8(std::ifstream& in, std::uint8_t& v)
{
    int c = in.get();
    if (c == EOF) return false;
    v = static_cast<std::uint8_t>(c);
    return true;
}

bool AuthService::read_blob(std::ifstream& in, std::string& s, std::uint32_t len)
{
    s.resize(len);
    in.read(s.data(), static_cast<std::streamsize>(len));
    return static_cast<bool>(in);
}

bool AuthService::get_key_from_file(const std::string& filename, std::uint8_t& key_out)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
    {
        std::cerr << "cannot open: " << filename << "\n";
        return false;
    }

    std::uint32_t file_magic = 0;
    if (!read_u32(in, file_magic))
    {
        std::cerr << "read magic failed\n";
        return false;
    }
    if (file_magic != magic)
    {
        std::cerr << "magic mismatch\n";
        return false;
    }

    std::uint8_t file_version = 0;
    if (!read_u8(in, file_version))
    {
        std::cerr << "read version failed\n";
        return false;
    }
    if (file_version != version)
    {
        std::cerr << "version mismatch\n";
        return false;
    }

    in.seekg(0, std::ios::end);
    auto size = in.tellg();
    if (size < 1)
    {
        std::cerr << "file empty\n";
        return false;
    }

    in.seekg(-1, std::ios::end);
    std::uint8_t key_obf_byte = 0;
    if (!read_u8(in, key_obf_byte))
    {
        std::cerr << "read tail key failed\n";
        return false;
    }

    key_out = static_cast<std::uint8_t>(key_obf_byte ^ key_mask);
    return true;
}

bool AuthService::idVerify(std::string i_user_name, std::string i_password)
{
    if (i_user_name.empty() || i_password.empty())
        return false;

    if (_key_path.trimmed().isEmpty())
    {
        std::cerr << "auth key path is not configured\n";
        return false;
    }

    std::string key_path = _key_path.toStdString();
    std::cout << key_path << std::endl;

    std::uint8_t key = 0;
    if (!get_key_from_file(key_path, key))
    {
        std::cerr << "get_key_from_file failed\n";
        return false;
    }

    std::ifstream in(key_path, std::ios::binary);
    if (!in)
    {
        std::cerr << "open key.dat failed\n";
        return false;
    }

    std::uint32_t file_magic = 0;
    if (!read_u32(in, file_magic)) return false;
    if (file_magic != magic)
    {
        std::cerr << "bad file magic\n";
        return false;
    }

    std::uint8_t file_version = 0;
    if (!read_u8(in, file_version)) return false;
    if (file_version != version)
    {
        std::cerr << "unsupported version\n";
        return false;
    }

    std::uint32_t u_len = 0, p_len = 0;

    if (!read_u32(in, u_len))
        return false;

    std::string u_enc_stored;
    if (!read_blob(in, u_enc_stored, u_len))
        return false;


    if (!read_u32(in, p_len))
        return false;

    std::string p_enc_stored;
    if (!read_blob(in, p_enc_stored, p_len))
        return false;


    const std::string u_enc_input = xor_copy(i_user_name, key);

    const std::string p_enc_input = xor_copy(i_password, key);

    std::cout << "u_enc_input : " << bytes_to_hex(u_enc_input) << std::endl;
    std::cout << "u_enc_stored: " << bytes_to_hex(u_enc_stored) << std::endl;
    std::cout << "p_enc_input : " << bytes_to_hex(p_enc_input) << std::endl;
    std::cout << "p_enc_stored: " << bytes_to_hex(p_enc_stored) << std::endl;

    const bool user_ok = (u_enc_input == u_enc_stored);
    const bool pass_ok = (p_enc_input == p_enc_stored);
    const bool verify_ok = user_ok && pass_ok;

    std::cout << "user_ok=" << user_ok
        << ", pass_ok=" << pass_ok
        << ", verify_ok=" << verify_ok << std::endl;

    return verify_ok;
}
