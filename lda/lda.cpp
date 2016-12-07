#include <iostream>
#include <fstream>
#include <assert.h>

#define NTOPICS 20
#define ALPHA   2.5
#define BETA    0.1

#define NEPOCHS 100

struct token_t{
  int wordid;
  int docid;
  int current_topic;
}; // each token is a pair of (word id,  doc id) and the current sampled topic.

struct topiccounts_t{
  int topiccounts[NTOPICS];
}; // count of each topic

int main(int argc, char** argv){

  // load meta data
  int n_tokens;// # distinct tokens
  int n_words; // # distinct words
  int n_docs;  // # distinct docs
  std::ifstream fin_dimensions("data/reuters21578/MINIMAL_worddict");
  fin_dimensions >> n_tokens >> n_words >> n_docs; 
  std::cout << "# NODES = " << n_tokens << std::endl;
  std::cout << "# EDGES = " << n_words + n_docs + 1 << std::endl;
  std::cout << "# LINKS = " << 3 * n_tokens << std::endl;

  // load data
  token_t * tokens = new token_t[n_tokens];
  std::ifstream fin_data("data/reuters21578/MINIMAL_wid_docid");
  int * tmp = (int *) tokens;
  for(int i=0;i<n_tokens*3;i+=3){ fin_data >> tmp[i] >> tmp[i+1]; }

  // allocate & initialize
  topiccounts_t * word_factors = new topiccounts_t[n_words];
  topiccounts_t * doc_factors = new topiccounts_t[n_docs];
  topiccounts_t all_factor;
  int * nwords_in_doc = new int[n_docs];
  for(int i=0;i<n_words;i++){ 
    for(int j=0;j<NTOPICS;j++){
      word_factors[i].topiccounts[j] = 0;
    }
  }
  for(int i=0;i<n_docs;i++){ 
    nwords_in_doc[i] = 0;
    for(int j=0;j<NTOPICS;j++){
      doc_factors[i].topiccounts[j] = 0;
    }
  }
  for(int j=0;j<NTOPICS;j++){
    all_factor.topiccounts[j] = 0;
  }
  for(int i=0;i<n_tokens;i++){ 
    tokens[i].current_topic = rand() % NTOPICS;
    word_factors[tokens[i].wordid].topiccounts[tokens[i].current_topic] ++;
    doc_factors[tokens[i].docid].topiccounts[tokens[i].current_topic] ++;
    nwords_in_doc[tokens[i].docid] ++;
    all_factor.topiccounts[tokens[i].current_topic] ++;
  }

  // start sample
  float topic_probs[NTOPICS];
  for(int iepoch=0;iepoch<NEPOCHS;iepoch++){
    std::cout << "START EPOCH " << iepoch << " ... " << std::flush;
    for(int i=0;i<n_tokens;i++){
      token_t & token = tokens[i];
      topiccounts_t & word_factorcounts = word_factors[token.wordid];
      topiccounts_t & doc_factorcounts = doc_factors[token.docid];

      float sum = 0.0;
      int calibration = 0;
      for(int j=0;j<NTOPICS;j++){ // these can be SIMD'ed, let's trust compiler
        calibration = (j == token.current_topic);
        float word_factor = word_factorcounts.topiccounts[j] - calibration + BETA;
        float doc_factor1 = doc_factorcounts.topiccounts[j] + ALPHA;
        float overall_factor = all_factor.topiccounts[j] - calibration + n_words * BETA;
        float doc_factor2 = nwords_in_doc[token.docid] - calibration + NTOPICS * ALPHA;
        topic_probs[j] = word_factor * doc_factor1 / overall_factor / doc_factor2;
        sum += topic_probs[j];
      }

      float r = drand48();
      float accumulate = 0;
      for(int j=0;j<NTOPICS;j++){
        accumulate += topic_probs[j]/sum;
        if(r <= accumulate){ // choose j as the new topics
          if(j == token.current_topic) break; // change nothing
          word_factorcounts.topiccounts[j] ++;
          word_factorcounts.topiccounts[token.current_topic] --;
          doc_factorcounts.topiccounts[j] ++;
          doc_factorcounts.topiccounts[token.current_topic] --;
          all_factor.topiccounts[j] ++;
          all_factor.topiccounts[token.current_topic] --;
          token.current_topic = j;
          break;
        }
      }
    }
    std::cout << "/DONE" << std::endl;
  }

  // output result
  for(int i=0;i<n_words;i++){
    int max = -1;
    int imax = -1;
    int sum = 0;
    for(int j=0;j<NTOPICS;j++){
      sum += word_factors[i].topiccounts[j];
      if(word_factors[i].topiccounts[j] >= max){
        max = word_factors[i].topiccounts[j];
        imax = j;
      }
    }
    std::cerr << i << " " << imax << " " << max << std::endl;
  }


  // release memory
  delete[] tokens;
  delete[] word_factors;
  delete[] doc_factors;
  delete[] nwords_in_doc;

  return 0;
}