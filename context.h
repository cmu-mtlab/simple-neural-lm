#pragma once
#include <list>
#include <functional>
#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
using namespace std;

class Context {
friend class std::hash<Context>;
public:
  Context() {}

  Context(unsigned size) {
    this->context_size = size;
  }
  void init(unsigned default_symbol) {
    context.clear();
    for (unsigned i = 0; i < context_size; ++i) {
      context.push_back(default_symbol);
    }
  }
  void add(unsigned word_id) {
    if (context.size() >= context_size) {
      context.pop_front();
    }
    context.push_back(word_id);
  }
  unsigned size() const {
    return context_size;
  }

  bool operator==(const Context& other) const {
    return this->context_size == other.context_size &&
           this->context == other.context;
  }

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & context_size;
    ar & context;
  }

  typedef list<unsigned>::iterator iterator;
  typedef list<unsigned>::const_iterator const_iterator;
  iterator begin() { return context.end(); }
  const_iterator begin() const { return context.begin(); }
  iterator end() { return context.end(); }
  const_iterator end() const { return context.end(); }
private:
  unsigned context_size;
  list<unsigned> context;
};

namespace std {
  template <class T>
  struct hash<list<T> > {
    size_t operator()(const list<T>& list) const {
      size_t seed = 0;
      for (T v : list) {
        hash_combine(seed, v);
      }
      return seed;
    }
  };

  template <>
  struct hash<Context> {
    size_t operator()(const Context& context) const {
      size_t seed = 0;
      hash_combine(seed, context.size());
      hash_combine(seed, context.context);
      return seed;
    }
  };
}
