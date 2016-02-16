#include "DataTypes.h"

std::ostream& operator<<( std::ostream& out, Token const& token) {
  return out << "Token - Port: " << token.port << " SeqNum: " << token.seq_num;
}

std::ostream& operator<<( std::ostream& out, Delta const& delta) {
  return out << "Delta - Scope: " << delta.scope << " Token: " << delta.token << " Count: " << delta.count;
}

std::ostream& operator<<( std::ostream& out, Sample const& sample) {
  return out << "Sample - Scope: " << sample.scope << " Token: " << sample.token << " Sample: " << sample.sample;
}

