#pragma once
#include <fstream>
#include <iostream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "adept.h"
#include "context.h"
#include "utils.h"
#include "param.h"

using namespace std;
using adept::adouble;

class NeuralLM {
public:
  NeuralLM(unsigned vocab_size, unsigned context_size, unsigned context_vector_size, unsigned output_vector_size, unsigned hidden_layer_size) {
    init(vocab_size, context_size, context_vector_size, output_vector_size, hidden_layer_size);
  }

  void init(unsigned vocab_size, unsigned context_size, unsigned context_vector_size, unsigned output_vector_size, unsigned hidden_layer_size) {
    context = Context(context_size);
    this->vocab_size = vocab_size;
    this->context_vector_size = context_vector_size;
    this->output_vector_size = output_vector_size;
    this->hidden_layer_size = hidden_layer_size;

    context_vectors.resize(vocab_size);
    output_vectors.resize(vocab_size);
    for (unsigned i = 0; i < vocab_size; ++i) {
      context_vectors[i].init(context_vector_size);
      output_vectors[i].init(output_vector_size);
    }
    hidden_layer_weights.init(hidden_layer_size, context_vector_size * context_size);
    output_layer_weights.init(output_vector_size, hidden_layer_size);
    output_bias.init(output_vector_size);
  }

  AColumnVector get_context_vector() const {
    return get_context_vector(this->context);
  }

  AColumnVector get_context_vector(const Context& context) const {
    AColumnVector v(context_size() * context_vector_size);
    unsigned i = 0;
    for (unsigned w : context) {
      for (unsigned j = 0; j < context_vector_size; ++j) {
        assert (i * context_vector_size + j < v.size());
        assert (w < context_vectors.size());
        v(i * context_vector_size + j) = context_vectors[w].var[j];
      }
      i++;
    }
    return v;
  }

  adouble log_prob(unsigned word_id) {
    return log_prob(this->context, word_id);
  }

  adouble log_prob(const Context& context, unsigned word_id) {
    AColumnVector input = get_context_vector(context);
    AColumnVector hidden = cwiseSigmoid(hidden_layer_weights.var * input);
    AColumnVector output = output_layer_weights.var * hidden + output_bias.var;

    vector<adouble> scores(vocab_size, 0.0);
    for (unsigned v = 0; v < vocab_size; ++v) {
      scores[v] = output.transpose() * output_vectors[v].var;
    }
    adouble sum = log_sum_exp(scores);
    return scores[word_id] - sum;
  }

  void reset_context(unsigned word_id) {
    context.init(word_id);
  }

  void update(double learning_rate) {
    for (unsigned v = 0; v < vocab_size; ++v) {
      context_vectors[v].update(learning_rate);
      output_vectors[v].update(learning_rate);
    }

    hidden_layer_weights.update(learning_rate);
    output_layer_weights.update(learning_rate);
    output_bias.update(learning_rate);
  }

  void add_to_context(unsigned word_id) {
    context.add(word_id);
  }

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) { 
    ar & context;
    ar & vocab_size;
    ar & hidden_layer_size;
    ar & context_vector_size;
    ar & output_vector_size;

    ar & context_vectors;
    ar & output_vectors;
    ar & hidden_layer_weights;
    ar & output_layer_weights;
    ar & output_bias;
  }

  static NeuralLM ReadFromFile(const string& filename) {
    NeuralLM lm;
    ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia & lm;
    return lm;
  }

  void WriteToFile(const string& filename) const {
    ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa & *this;
  }

  unsigned context_size() const {
    return context.size();
  }

private:
  NeuralLM() {}

  Context context;
  unsigned vocab_size;
  unsigned hidden_layer_size;
  unsigned context_vector_size;
  unsigned output_vector_size;

  vector<Param<AColumnVector, ColumnVector> > context_vectors;
  vector<Param<AColumnVector, ColumnVector> > output_vectors;
  Param<AMatrix, Matrix> hidden_layer_weights;
  Param<AMatrix, Matrix> output_layer_weights;
  Param<AColumnVector, ColumnVector> output_bias; 
};

