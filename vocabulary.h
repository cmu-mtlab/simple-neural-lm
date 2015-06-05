#pragma once
#include <map>
#include <string>
#include <vector>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
using std::vector;
using std::map;
using std::string;

class vocabulary {
public:
  vocabulary();
  unsigned convert(string word);
  unsigned lookup(string word, unsigned unk);
  string convert(unsigned id);
  bool contains(string word);
  unsigned size() const;
  void lock();

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & words;
    ar & indices;
    ar & locked;
  }
  static vocabulary ReadFromFile(const string& filename);
  void WriteToFile(const string& filename) const;

private:
  vector<string> words;
  map<string, unsigned> indices;
  bool locked;
};
