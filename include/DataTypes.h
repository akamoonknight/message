#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#include <cstdint>
#include <ostream>

// Copies of the base types

struct Token
{
  std::uint16_t port = 0;
  std::uint64_t seq_num = 0;

  Token() {}

  Token( std::uint16_t port, std::uint64_t seq_num)
    : port( port)
    , seq_num( seq_num)
  {}
};

struct Delta
{
  std::uint16_t scope = 0;
  Token         token;
  std::uint32_t count = 0;

  Delta() {}

  Delta( std::uint16_t scope, Token token, std::uint32_t count)
    : scope( scope)
    , token( token)
    , count( count)
  {}
};

struct Sample
{
  std::uint16_t scope = 0;
  Token         token;
  std::uint64_t sample = 0;

  Sample() {}

  Sample( std::uint16_t scope, Token token, std::uint64_t sample)
    : scope( scope)
    , token( token)
    , sample( sample)
  {}
};

std::ostream& operator<<( std::ostream& out, Token const& token);
std::ostream& operator<<( std::ostream& out, Delta const& delta);
std::ostream& operator<<( std::ostream& out, Sample const& sample);

#endif // __DATA_TYPES_H__
