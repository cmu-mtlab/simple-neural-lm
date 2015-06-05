#pragma once
#include "adept.h"

template <class AT, class T>
class Param {
public:
  AT var;
  T gradient_sum;
  T delta_sum;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & var;
    ar & gradient_sum;
    ar & delta_sum;
  }

  void init(const int rows = 1, const int cols = 1) {
    var = AT::Random(rows, cols);
    gradient_sum = T::Zero(rows, cols);
    delta_sum = T::Zero(rows, cols);
  }

  void sgd_update(double learning_rate) {
    for (unsigned i = 0; i < var.rows(); ++i) {
      for (unsigned j = 0; j < var.cols(); ++j) {
        double g = var(i, j).get_gradient();
        var(i, j) -= g * learning_rate;
      }
    }
  }

  void adagrad_update(double learning_rate) {
    const double epsilon = 1.0e-100;
    for (unsigned i = 0; i < var.rows(); ++i) {
      for (unsigned j = 0; j < var.cols(); ++j) {
        double g = var(i, j).get_gradient();
        gradient_sum(i, j) += g * g;
        double denominator = sqrt(gradient_sum(i, j)) + epsilon;
        var(i, j) -= g * learning_rate / denominator;
      }
    }
  }

  void adadelta_update() {
    const double nepsilon = 1.0e-8;
    const double depsilon = 1.0e-100;
    const double rho = 0.95;
    for (unsigned i = 0; i < var.rows(); ++i) {
      for (unsigned j = 0; j < var.cols(); ++j) {
        double g = var(i, j).get_gradient();
        gradient_sum(i, j) = rho * gradient_sum(i, j) + (1.0 - rho) * g * g;
        double numerator = g * sqrt(delta_sum(i, j) + nepsilon);
        double denominator = sqrt(gradient_sum(i, j) + depsilon);
        double delta = numerator / denominator;
        delta_sum(i, j) = rho * delta_sum(i, j) + (1.0 - rho) * delta * delta;
        var(i, j) -= delta;
      }
    } 
  }

  void update(double learning_rate) {
    adadelta_update();
  }
};

