#include <set>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <cassert>
#include <fstream>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#define EIGEN_DENSEBASE_PLUGIN "EigenDenseBaseAddons.h"
#include "adept.h"
#include "vocabulary.h"
#include "utils.h"
#include "param.h"
#include "context.h"
#include "neurallm.h"

using namespace std;
using adept::adouble;

adept::Stack stack;
NeuralLM* model;
vocabulary vocab;
const int context_size = 4;
const int hidden_layer_size = 100;
const int context_vector_size = 10;
const int output_vector_size = 10;
const double learning_rate = 0.01;

void read_input_file(string filename, vocabulary& vocab, vector<vector<unsigned> >& lines) {
  ifstream f(filename);
  if (!f.is_open()) {
    cerr << "Unable to read from file " << filename << "." << endl;
    exit(1);
  }

  string line_in;
  vector<unsigned> line;
  vector<string> source;
  while (getline(f, line_in)) {
    stringstream sstream(line_in);
    
    string word;
    while (sstream >> word) {
      line.push_back(vocab.convert(word));
    }
    lines.push_back(line);
    line.clear();
  }
}

void exception_handler(int sig) {
  void *array[10];
  int size = backtrace(array, 10);

  cerr << "Error: signal " << sig << ":" << endl;
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void ctrlc_handler(int sig) {
 cerr << "Dumping model ..." << endl;
 if (model != NULL) {
    model->WriteToFile("model.nlm");
    vocab.WriteToFile("model.vcb");
    delete model;
  }
  model = NULL;
  exit(1);
}

int main(int argc, char** argv) { 
  signal(SIGSEGV, exception_handler);
  signal(SIGINT, ctrlc_handler);
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " train.txt\n";
    return 1;
  }
  srand(0);

  vocab.convert("<unk>");
  vocab.convert("<s>");
  vocab.convert("</s>");

  // read training data
  vector<vector<unsigned> > sentences;
  read_input_file(argv[1], vocab, sentences);
  cerr << "Successfully read " << sentences.size() << " training sentences." << endl;
  cerr << "Vocabulary size: " << vocab.size() << " (including <s>, </s>, and <unk>)" << endl;
  vocab.lock();

  model = new NeuralLM(vocab.size(), context_size, context_vector_size, output_vector_size, hidden_layer_size);

  adouble loss = 0.0;
  adouble log_prob = 0.0;
  adouble total_loss = 0.0;
  adouble perplexity = 0.0;
  int word_count = 0;

  for (unsigned i = 0; i < 10; ++i) {
    total_loss = 0.0;
    perplexity = 0.0;
    word_count = 0.0;
    for (unsigned j = 0; j < sentences.size(); ++j) { 
      model->reset_context(vocab.convert("<s>"));
      adouble sent_loss = 0.0; 
      for (unsigned k = 0; k < sentences[j].size() + 1; ++k) {
        stack.new_recording();
        unsigned w = (k < sentences[j].size()) ? sentences[j][k] : vocab.convert("</s>");
        log_prob = model->log_prob(w);
        loss = -log_prob;
        sent_loss += loss;
        total_loss += loss;

        model->add_to_context(w);
        word_count++;

        loss.set_gradient(1.0);
        stack.compute_adjoint();
        if (i > 0) {
          model->update(learning_rate);
        }
      }
    }
    cerr << "loss after " << i << " iterations: " << total_loss << ", perplexity: " << total_loss / word_count << endl;
  }

  if (model != NULL) {
    model->WriteToFile("model.nlm");
    vocab.WriteToFile("model.vcb");
    delete model;
  }
  model = NULL; 

}
