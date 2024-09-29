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

  if (!text.empty()) {
    positions_.insert(pos);
  }

  if (!table_.count(pos)) {
    table_[pos] = std::make_unique<Cell>(*this, pos, std::move(text));
  } else {
    table_[pos]->Set(std::move(text));
  }
}

bool Sheet::IsCellAvailable(Position pos) const {
  Size size_area = GetPrintableSize();
  return pos.IsValid() ? pos.row < size_area.rows && pos.col < size_area.cols
                       : false;
}

const CellInterface *Sheet::GetCell(Position pos) const {
  return const_cast<Sheet *>(this)->GetCell(pos);
}

CellInterface *Sheet::GetCell(Position pos) {
  if (!pos.IsValid())
    throw InvalidPositionException("Sheet::GetCell: Invalid position");

  if (IsCellAvailable(pos)) {
    if (!table_.count(pos)) {
      SetCell(pos, std::string());
    }

    return table_.at(pos).get();
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
    table_.erase(pos);
  }
  positions_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
  if (table_.empty()) {
    return Size{}; // Если таблица пуста, возвращаем размер (0, 0)
  }

  // Инициализируем минимальные и максимальные значения
  int min_row = std::numeric_limits<int>::max();
  int max_row = std::numeric_limits<int>::min();
  int min_col = std::numeric_limits<int>::max();
  int max_col = std::numeric_limits<int>::min();

  bool has_non_empty_cells = false;

  for (const auto &[position, cell_ptr] : table_) {
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
      if (table_.count(pos) > 0) {
        if (std::holds_alternative<double>(table_.at(pos)->GetValue())) {
          output << std::get<double>(table_.at(pos)->GetValue());
        } else if (std::holds_alternative<std::string>(
                       table_.at(pos)->GetValue())) {
          output << std::get<std::string>(table_.at(pos)->GetValue());
        } else {
          output << std::get<FormulaError>(table_.at(pos)->GetValue());
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
      if (table_.count(pos) > 0) {
        output << table_.at(pos)->GetText();
      }
    }

    output << '\n';
  }
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}