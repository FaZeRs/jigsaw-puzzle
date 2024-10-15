#include <algorithm>
#include <filesystem>
#include <optional>
#include <ranges>
#include <stack>
#include <span>
#include <iostream>
#include <fstream>
#include <libgen.h>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "timer.h"

constexpr int PUZZLE_GRID_SIZE = 16;
constexpr int PUZZLE_SIZE = 256;
constexpr const char* ASSETS_DIR = "assets_png";
constexpr double FIRST_COL_WIDTH = 240.0;
constexpr double FIRST_ROW_HEIGHT = 135.0;
constexpr int IMAGE_WIDTH = 3840;
constexpr int IMAGE_HEIGHT = 2160;
constexpr size_t HASH_MAGIC_NUMBER = 0x9e379967;

enum class Side { LEFT, RIGHT, TOP, BOTTOM };
constexpr std::array<std::pair<int, int>, 4> offsets = {
    std::pair{-1, 0}, // Side::LEFT
    std::pair{1, 0}, // Side::RIGHT
    std::pair{0, -1}, // Side::TOP
    std::pair{0, 1} // Side::BOTTOM
};

struct PuzzlePiece {
  PuzzlePiece() = default;
  ~PuzzlePiece() = default;
  PuzzlePiece(const PuzzlePiece&) = default;
  PuzzlePiece& operator=(const PuzzlePiece&) = default;
  PuzzlePiece(PuzzlePiece&&) noexcept = default;
  PuzzlePiece& operator=(PuzzlePiece&&) noexcept = default;
  bool operator<(const PuzzlePiece& other) const {
    if ((col == 0 || row == 0) && (other.col != 0 && other.row != 0)) {
      return true;
    }
    if ((other.col == 0 || other.row == 0) && (col != 0 && row != 0)) {
      return false;
    }
    if (col == 0 && row == 0) {
      return true;
    }
    if (other.col == 0 && other.row == 0) {
      return false;
    }
    return (col + row) < (other.col + other.row);
  }
  size_t id;
  cv::Mat image;
  int col{-1};
  int row{-1};
  size_t left_hash;
  size_t right_hash;
  size_t top_hash;
  size_t bottom_hash;

  cv::Rect rect() const {
    const auto top_left =
        cv::Point2f(FIRST_COL_WIDTH * col - (col > 0 ? 1.0 : 0.0), FIRST_ROW_HEIGHT * row - (row > 0 ? 1.0 : 0.0));
    return cv::Rect(top_left, cv::Size(image.cols, image.rows));
  }
  void move(const int source_col, const int source_row, const Side side) {
    const auto& [col_offset, row_offset] = offsets[static_cast<int>(side)];
    col = source_col + col_offset;
    row = source_row + row_offset;
  }
};

size_t computeHash(const cv::Mat& edge) {
  size_t hash = 0;
  for (int row = 0; row < edge.rows; ++row) {
    const uchar* data = edge.ptr<uchar>(row);
    for (int col = 0; col < edge.cols; ++col) {
      hash ^= (data[col] / 10) + HASH_MAGIC_NUMBER + (hash << 6) + (hash >> 2);
    }
  }
  return hash;
}

void computeEdgeHashes(PuzzlePiece& piece) {
  cv::Mat gray;
  cv::cvtColor(piece.image, gray, cv::COLOR_BGR2GRAY);

  piece.left_hash = computeHash(gray.col(0));
  piece.right_hash = computeHash(gray.col(gray.cols - 1));
  piece.top_hash = computeHash(gray.row(0));
  piece.bottom_hash = computeHash(gray.row(gray.rows - 1));
}

class PuzzlePieceProcessor : public cv::ParallelLoopBody {
 public:
  PuzzlePieceProcessor(const std::span<const std::string>& file_paths, const std::span<PuzzlePiece>& pieces) :
      file_paths_(file_paths), pieces_(pieces) {}

  void operator()(const cv::Range& range) const override {
    for (int i = range.start; i < range.end; ++i) {
      cv::Mat image = cv::imread(file_paths_[i], cv::IMREAD_COLOR);
      auto& piece = pieces_[i];
      piece.id = i;
      piece.image = std::move(image);
      computeEdgeHashes(piece);
    }
  }

 private:
  const std::span<const std::string>& file_paths_;
  std::span<PuzzlePiece> pieces_;
};

[[nodiscard]] std::array<PuzzlePiece, PUZZLE_SIZE> loadPuzzle(const std::string_view& path) {
  std::vector<std::string> file_paths;
  file_paths.reserve(PUZZLE_SIZE);
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    file_paths.push_back(entry.path());
  }

  std::array<PuzzlePiece, PUZZLE_SIZE> pieces;
  cv::parallel_for_(cv::Range(0, static_cast<int>(file_paths.size())), PuzzlePieceProcessor(file_paths, pieces),
                    cv::getNumThreads());

  return pieces;
}

void findStartingPieces(std::array<PuzzlePiece, PUZZLE_SIZE>& pieces) {
  for (auto& piece : pieces) {
    if (piece.image.cols == FIRST_COL_WIDTH && piece.image.rows == FIRST_ROW_HEIGHT) {
      piece.col = 0;
      piece.row = 0;
    } else if (piece.image.cols == FIRST_COL_WIDTH) {
      piece.col = 0;
    } else if (piece.image.rows == FIRST_ROW_HEIGHT) {
      piece.row = 0;
    }
  }
}

size_t getHash(const PuzzlePiece& piece, const Side side) {
  static constexpr std::array<size_t PuzzlePiece::*, 4> hashes = {&PuzzlePiece::left_hash, &PuzzlePiece::right_hash,
                                                                  &PuzzlePiece::top_hash, &PuzzlePiece::bottom_hash};
  return piece.*hashes[static_cast<int>(side)];
}

PuzzlePiece* findMatch(const PuzzlePiece& source_piece, const std::unordered_multimap<size_t, PuzzlePiece*>& hash_map,
                       const Side side) {
  const size_t source_hash = getHash(source_piece, side);
  auto [matches_begin, matches_end] = hash_map.equal_range(source_hash);

  for (auto it = matches_begin; it != matches_end; ++it) {
    auto& candidate = it->second;

    if (source_piece.id == candidate->id) {
      continue;
    }
    if (candidate->col != -1 && candidate->row != -1) {
      continue;
    }

    candidate->move(source_piece.col, source_piece.row, side);
    return candidate;
  }

  return nullptr;
}

std::unordered_multimap<size_t, PuzzlePiece*> buildHashMap(std::array<PuzzlePiece, PUZZLE_SIZE>& pieces) {
  std::unordered_multimap<size_t, PuzzlePiece*> hash_map;
  hash_map.reserve(PUZZLE_SIZE * 4);
  for (auto& piece : pieces) {
    hash_map.emplace(piece.left_hash, &piece);
    hash_map.emplace(piece.right_hash, &piece);
    hash_map.emplace(piece.top_hash, &piece);
    hash_map.emplace(piece.bottom_hash, &piece);
  }
  return hash_map;
}

void assemblePuzzle(std::array<PuzzlePiece, PUZZLE_SIZE>& pieces) {
  std::sort(pieces.begin(), pieces.end());

  const auto hash_map = buildHashMap(pieces);

  std::stack<PuzzlePiece*> stack;
  stack.push(&pieces[0]);

  while (!stack.empty()) {
    const auto current_piece = stack.top();
    stack.pop();

    if (current_piece->col < PUZZLE_GRID_SIZE) {
      auto right_match = findMatch(*current_piece, hash_map, Side::RIGHT);
      if (right_match && right_match->col < PUZZLE_GRID_SIZE) {
        stack.push(right_match);
      }
    }

    if (current_piece->row < PUZZLE_GRID_SIZE) {
      auto bottom_match = findMatch(*current_piece, hash_map, Side::BOTTOM);
      if (bottom_match && bottom_match->row < PUZZLE_GRID_SIZE) {
        stack.push(bottom_match);
      }
    }
  }
}

class PuzzleStich : public cv::ParallelLoopBody {
 public:
  PuzzleStich(const std::span<const PuzzlePiece>& pieces, cv::Mat& result) : pieces_(pieces), result_(result) {}

  void operator()(const cv::Range& range) const override {
    for (int i = range.start; i < range.end; i++) {
      if (pieces_[i].col == -1 || pieces_[i].row == -1) {
        continue;
      }
      if (pieces_[i].col >= PUZZLE_GRID_SIZE || pieces_[i].row >= PUZZLE_GRID_SIZE) {
        continue;
      }
      pieces_[i].image.copyTo(result_(pieces_[i].rect()));
    }
  }

 private:
  const std::span<const PuzzlePiece> pieces_;
  cv::Mat& result_;
};

int main(int argc, char** argv) {
  Timer total_timer;
  Timer timer;

  const char* dir = dirname(argv[0]);
  const std::string assets_path = std::string(dir) + "/" + ASSETS_DIR;
  std::array puzzle_pieces = loadPuzzle(assets_path);

  std::cout << "Load puzzle time: " << timer.elapsedMs() << "\n";
  timer.reset();

  findStartingPieces(puzzle_pieces);

  std::cout << "Find starting piece time: " << timer.elapsedMs() << "\n";
  timer.reset();

  assemblePuzzle(puzzle_pieces);

  std::cout << "Assemble puzzle time: " << timer.elapsedMs() << "\n";
  timer.reset();

  auto result = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
  cv::parallel_for_(cv::Range(0, puzzle_pieces.size()), PuzzleStich(puzzle_pieces, result), cv::getNumThreads());
  std::cout << "Image creation time: " << timer.elapsedMs() << "\n";
  timer.reset();

  cv::imwrite("result.jpg", result, {cv::IMWRITE_JPEG_QUALITY, 80});

  std::cout << "Image write time: " << timer.elapsedMs() << "\n";
  std::cout << "Total time: " << total_timer.elapsedMs() << "\n";

  return 0;
}