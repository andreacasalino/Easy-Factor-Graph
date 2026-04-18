/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/factor/Transform.h>

#include "Printing.h"

void Tabular::addLine(Line line) {
  columns_max_len.resize(line.sizes.size(), 0);
  for (std::size_t c = 0; c < line.sizes.size(); ++c) {
    columns_max_len[c] =
        std::max<std::size_t>(columns_max_len[c], line.sizes[c]);
  }
  lines.emplace_back(std::move(line));
}

void Tabular::print(std::ostream &s) const {
  for (const auto &line : lines) {
    s << '\n';

    auto print_ = [&](std::string_view to_add, std::size_t len_tot) {
      s << to_add;
      std::size_t delta = len_tot - to_add.size();
      for (std::size_t i = 0; i < delta; ++i) {
        s << ' ';
      }
    };

    print_(line.get(0), columns_max_len[0]);
    for (std::size_t c = 1; c < line.sizes.size(); ++c) {
      s << ' ';
      print_(line.get(c), columns_max_len[c]);
    }
  }
}

std::string_view Tabular::Line::get(std::size_t idx) const {
  if (idx == 0) {
    return {buffer_.begin(), buffer_.begin() + sizes.front()};
  } else {
    std::size_t offset{0};
    for (std::size_t c = 0; c < idx; ++c) {
      offset += sizes[c];
    }
    auto it_begin = buffer_.begin() + offset;
    auto it_end = it_begin + sizes[idx];
    return {it_begin, it_end};
  }
}

template <std::size_t N, typename Transform>
std::ostream &print(std::ostream &s,
                    const EFG::factor::FactorT<N, Transform> &distribution) {
  Tabular table;
  table.addLine(
      Tabular::Line{"combination", "", "raw image value   ", "image value"});
  if constexpr (N == 1) {
    std::size_t k{0};
    for (auto val : distribution.getAllValues()) {
      table.addLine(Tabular::Line{std::size_t{k++}, " -> ", val,
                                  distribution.trsfm(val)});
    }
  } else {
    std::string comb_buffer;
    distribution.template forEachCombination<false>(
        [&](const EFG::categoric::Combination<N> &comb, float val) {
          comb_buffer = "[";
          comb_buffer += std::to_string(comb[0]);
          std::for_each(comb.begin() + 1, comb.end(), [&comb_buffer](auto val) {
            comb_buffer.push_back(' ');
            comb_buffer += std::to_string(val);
          });
          comb_buffer.push_back(']');

          table.addLine(Tabular::Line{
              std::string_view{comb_buffer.begin(), comb_buffer.end()}, " -> ",
              val, distribution.trsfm(val)});
        });
  }
  table.print(s);
  return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template std::ostream &print<1, EFG::factor::NullTrasform>(
    std::ostream &, const EFG::factor::FactorT<1, EFG::factor::NullTrasform> &);

template std::ostream &print<1, EFG::factor::ExponentialTrasform>(
    std::ostream &,
    const EFG::factor::FactorT<1, EFG::factor::ExponentialTrasform> &);

template std::ostream &print<2, EFG::factor::NullTrasform>(
    std::ostream &, const EFG::factor::FactorT<2, EFG::factor::NullTrasform> &);

template std::ostream &print<2, EFG::factor::ExponentialTrasform>(
    std::ostream &,
    const EFG::factor::FactorT<2, EFG::factor::ExponentialTrasform> &);

template std::ostream &print<3, EFG::factor::NullTrasform>(
    std::ostream &, const EFG::factor::FactorT<3, EFG::factor::NullTrasform> &);

template std::ostream &print<3, EFG::factor::ExponentialTrasform>(
    std::ostream &,
    const EFG::factor::FactorT<3, EFG::factor::ExponentialTrasform> &);

template std::ostream &print<4, EFG::factor::NullTrasform>(
    std::ostream &, const EFG::factor::FactorT<4, EFG::factor::NullTrasform> &);

template std::ostream &print<4, EFG::factor::ExponentialTrasform>(
    std::ostream &,
    const EFG::factor::FactorT<4, EFG::factor::ExponentialTrasform> &);