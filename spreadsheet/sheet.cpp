#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
  if (!pos.IsValid())
    throw InvalidPositionException("Sheet::SetCell: Invalid position");

  if (!cells_.count(pos)) {
    std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this, pos);
    cell -> Set(std::move(text));
    cells_[pos] = std::move(cell);
  } else {
    cells_[pos]->Set(std::move(text));
  }
}

bool Sheet::IsCellAvailable(Position pos) const {
  Size size_area = GetPrintableSize();
  return pos.IsValid() ? pos.row < size_area.rows && pos.col < size_area.cols
                       : false;
}

const CellInterface *Sheet::GetCell(Position pos) const {
  return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface *Sheet::GetCell(Position pos) {
  if (!pos.IsValid())
    throw InvalidPositionException("Sheet::GetCell: Invalid position");

  if (IsCellAvailable(pos)) {
    if (!cells_.count(pos)) {
      SetCell(pos, std::string());
    }

    return cells_.at(pos).get();
  } else {
    return nullptr;
  }
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
  return const_cast<Sheet*>(this)->GetConcreteCell(pos);
} 

Cell* Sheet::GetConcreteCell(Position pos){
  if (!pos.IsValid())
    throw InvalidPositionException("Sheet::GetConcreteCell: Invalid position");

  if (IsCellAvailable(pos)) {
    if (!cells_.count(pos)) {
      return nullptr;
    }

    return cells_.at(pos).get();
  } else {
    return nullptr;
  }
} 

void Sheet::ClearCell(Position pos) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Sheet::ClearCell: Invalid position");
  }
  if (IsCellAvailable(pos)) {
    SetCell(pos, std::string());
  } else {
    cells_.erase(pos);
  }
}

Size Sheet::GetPrintableSize() const {
  if (cells_.empty()) {
    return Size{}; // Если таблица пуста, возвращаем размер (0, 0)
  }

  // Инициализируем минимальные и максимальные значения
  int min_row = std::numeric_limits<int>::max();
  int max_row = std::numeric_limits<int>::min();
  int min_col = std::numeric_limits<int>::max();
  int max_col = std::numeric_limits<int>::min();

  bool has_non_empty_cells = false;

  for (const auto &[position, cell_ptr] : cells_) {
    if (cell_ptr &&
        !cell_ptr->GetText()
             .empty()) { // Учитываем только ячейки с непустым текстом
      has_non_empty_cells = true;
      min_row = std::min(min_row, position.row);
      max_row = std::max(max_row, position.row);
      min_col = std::min(min_col, position.col);
      max_col = std::max(max_col, position.col);
    }
  }

  if (!has_non_empty_cells) {
    return Size{}; // Если нет ячеек с непустым текстом, возвращаем (0, 0)
  }

  // Возвращаем размер минимальной печатной области
  return {max_row + 1,
          max_col + 1}; // Индексы начинаются с 0, поэтому добавляем 1
}

void Sheet::PrintValues(std::ostream &output) const {
  for (int y = 0; y < GetPrintableSize().rows; ++y) {
    for (int x = 0; x < GetPrintableSize().cols; ++x) {
      if (x > 0) {
        output << '\t';
      }
      Position pos{y, x};
      if (cells_.count(pos) > 0) {
        if (std::holds_alternative<double>(cells_.at(pos)->GetValue())) {
          output << std::get<double>(cells_.at(pos)->GetValue());
        } else if (std::holds_alternative<std::string>(
                       cells_.at(pos)->GetValue())) {
          output << std::get<std::string>(cells_.at(pos)->GetValue());
        } else {
          output << std::get<FormulaError>(cells_.at(pos)->GetValue());
        }
      }
    }

    output << '\n';
  }
}
void Sheet::PrintTexts(std::ostream &output) const {
  for (int y = 0; y < GetPrintableSize().rows; ++y) {
    for (int x = 0; x < GetPrintableSize().cols; ++x) {
      if (x > 0) {
        output << '\t';
      }
      Position pos{y, x};
      if (cells_.count(pos) > 0) {
        output << cells_.at(pos)->GetText();
      }
    }

    output << '\n';
  }
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}