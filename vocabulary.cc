#include <cassert>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "vocabulary.h"
using namespace std;

vocabulary::vocabulary() {
  locked = false;
}

unsigned vocabulary::convert(string word) {
  if (indices.find(word) == indices.end()) {
    if (locked) {
      cerr << "word = " << word << endl;
      assert ("Invalid attempt to add word to a locked vocabulary!" && false);
    }
    indices[word] = words.size();
    words.push_back(word);
  }
  return indices[word];
}

unsigned vocabulary::lookup(string word, unsigned unk) {
  if (indices.find(word) == indices.end()) {
   return unk;
  }
  return indices[word];
}

string vocabulary::convert(unsigned id) {
  assert (id >= 0);
  assert (id < words.size());
  return words[id];
}

bool vocabulary::contains(string word) {
  return (indices.find(word) != indices.end());
}

unsigned vocabulary::size() const {
  return words.size();
}

void vocabulary::lock() {
  locked = true;
}

vocabulary vocabulary::ReadFromFile(const string& filename) {
  vocabulary v;
  ifstream ifs(filename);
  boost::archive::text_iarchive ia(ifs);
  ia & v;
  return v;
}

void vocabulary::WriteToFile(const string& filename) const {
  ofstream ofs(filename);
  boost::archive::text_oarchive oa(ofs);
  oa & *this;
}

