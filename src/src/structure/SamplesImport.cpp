/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/File.h>
#include <EasyFactorGraph/structure/SamplesImport.h>

namespace EFG::structure {
namespace {
std::size_t parse_sample_len(const char *val) {
  std::size_t res{0};
  int i{0};
  for (int k = 7; 0 <= k; --k, ++i) {
    res |= val[i] << k * 8;
  }
  return res;
}

categoric::VarStateSize parse_value(const char *val) {
  categoric::VarStateSize res{0};
  res |= val[0] << 8;
  res |= val[1];
  return res;
}

struct StreamReadGen {
  StreamReadGen(std::unique_ptr<std::ifstream> fd) : fd_{std::move(fd)} {}

  std::optional<std::string_view> read_next(std::size_t len) {
    buffer_.clear();
    buffer_.resize(len, 0);
    fd_->read(buffer_.data(), len);
    std::streamsize bytesRead = fd_->gcount();
    if (bytesRead == 0) {
      // No more bytes are available (EOF reached cleanly before reading
      // anything)
      return std::nullopt;
    } else if (bytesRead < len) {
      // Partial read: fewer than N bytes available
      throw Error{"Invalid samples file"};
    }
    // Success: Exactly N bytes were read
    return std::string_view{buffer_.data(), len};
  }

private:
  std::vector<char> buffer_;
  std::unique_ptr<std::ifstream> fd_;
};
} // namespace

std::unique_ptr<misc::Samples>
load_train_set(const std::filesystem::path &file_name,
               std::size_t samples_per_buffer) {
  StreamReadGen fd{misc::open<misc::FileMode::READ>(file_name)};

  std::size_t sample_len;
  if (auto val = fd.read_next(8); val.has_value()) {
    sample_len = parse_sample_len(val->data());
  } else {
    throw Error{"Invalid samples file"};
  }

  std::vector<categoric::VarStateSize> sample_buffer;

  auto res = std::make_unique<misc::Samples>(sample_len, samples_per_buffer);
  while (true) {
    auto maybe_next =
        fd.read_next(sample_len * sizeof(categoric::VarStateSize));
    if (!maybe_next.has_value()) {
      break;
    }

    std::size_t cursor{0};
    sample_buffer.clear();
    for (std::size_t i = 0; i < sample_len;
         ++i, cursor += sizeof(categoric::VarStateSize)) {
      sample_buffer.push_back(parse_value(maybe_next->data() + cursor));
    }
    res->add(sample_buffer);
  }
  return res;
}

void dump_train_set(const misc::Samples &samples,
                    const std::filesystem::path &file_name) {
  auto stream = misc::open<misc::FileMode::WRITE>(file_name);

  auto sample_len = samples.eachSampleSize();
  for (int k = 7; 0 <= k; --k) {
    *stream << static_cast<char>(sample_len >> k * 8);
  }

  auto it = samples.makeIter();
  while (true) {
    auto maybe_sample = it.next();
    if (maybe_sample.has_value()) {
      for (auto val : *maybe_sample) {
        *stream << static_cast<char>(val >> 8);
        *stream << static_cast<char>(val);
      }
    } else {
      break;
    }
  }
}
} // namespace EFG::structure
