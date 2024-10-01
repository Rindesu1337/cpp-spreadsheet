#include "cell.h"
#include "sheet.h"

Cell::Cell(Sheet &sheet, Position position)
    : sheet_(sheet), impl_(std::make_unique<EmptyImpl>()), position_(position) {
}

void Cell::Set(std::string text) {
  std::unique_ptr<Impl> temp_impl;

  if (text.empty()) {
    temp_impl = std::make_unique<EmptyImpl>();
  } else if (text.size() >= 2 && text.at(0) == FORMULA_SIGN) {
    temp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
  } else {
    temp_impl = std::make_unique<TextImpl>(std::move(text));
  }

  if (CircularDependency(temp_impl)) {
    throw CircularDependencyException("Cyclic dependency detected");
  }

  NewReference(temp_impl->GetReferencedCells());
  InvalidCache();

  impl_ = std::move(temp_impl);
}

void Cell::NewReference(const std::vector<Position> &new_dependents) {
  for (Cell *cell : dependent_cells_) {
    if (!cell) {
      continue;
    }

    cell->dependent_cells_.erase(this);
  }

  referenced_cells_.clear();
  if (!new_dependents.empty()) {
    for (auto &&pos : new_dependents) {
      Cell *cell = sheet_.GetConcreteCell(pos);
      referenced_cells_.insert(cell);
    }
  }

  for (Cell *cell : referenced_cells_) {
    if (!cell) {
      continue;
    }

    dependent_cells_.insert(cell);
  }
}

void Cell::InvalidCache() {
  impl_->InvalidateCache();

  for (Cell *cell : referenced_cells_) {
    if (!cell) {
      continue;
    }
    if (cell->impl_->IsCacheValid()) {
      cell->InvalidCache();
    }
  }
}

void Cell::Clear() { impl_ = std::make_unique<EmptyImpl>(); }

Cell::Value Cell::GetValue() const { return impl_->GetValue(); }

std::string Cell::GetText() const { return impl_->GetText(); }

std::vector<Position> Cell::GetReferencedCells() const {
  return impl_->GetReferencedCells();
}

bool Cell::CircularDependency(
    std::unique_ptr<Cell::Impl> &impl) const {
  auto positions = impl->GetReferencedCells();
  PositionsSet dependents(positions.begin(), positions.end());
  PositionsSet checkeds;
  return DFS(dependents, checkeds);
}

bool Cell::DFS(const PositionsSet &dependents,
               PositionsSet &verified) const {
  // Если текущая ячейка уже есть среди зависимых, значит, цикл найден
  if (dependents.count(position_)) {
    return true;
  }
  for (Position pos : dependents) {
    if (pos.IsValid() && !verified.count(pos)) {
      verified.insert(pos);
      auto cell = sheet_.GetCell(pos);
      if (cell) {
        auto ref = cell->GetReferencedCells();
        if (DFS({ref.begin(), ref.end()}, verified)) {
          return true;
        }
      }
    }
  }
  return false;
}
///////////////////////////

bool Cell::Impl::IsCacheValid() const { return false; }

void Cell::Impl::InvalidateCache() const {}

////////////////////////////

Cell::Value Cell::EmptyImpl::GetValue() const { return std::string(); }

std::string Cell::EmptyImpl::GetText() const { return std::string(); }

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const { return {}; }

////////////////////////////////

Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
  if (!text_.empty() && text_.at(0) == ESCAPE_SIGN) {
    return text_.substr(1);
  } else {
    return text_;
  }
}

std::string Cell::TextImpl::GetText() const { return text_; }

std::vector<Position> Cell::TextImpl::GetReferencedCells() const { return {}; }

//////////////////////////////

Cell::FormulaImpl::FormulaImpl(std::string text, const SheetInterface &sheet)
    : sheet_(sheet), formula_(ParseFormula(text.substr(1))) // Обрезаем '='

{}

Cell::Value Cell::FormulaImpl::GetValue() const {
  if (IsCacheValid()) {
    if (std::holds_alternative<double>(cache_.value())) {
      return std::get<double>(cache_.value());
    } else {
      return std::get<FormulaError>(cache_.value());
    }
  }

  cache_ = formula_->Evaluate(sheet_);
  if (std::holds_alternative<double>(cache_.value())) {
    return std::get<double>(cache_.value());
  } else {
    return std::get<FormulaError>(cache_.value());
  }
}

std::string Cell::FormulaImpl::GetText() const {
  return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
  return formula_->GetReferencedCells();
}

bool Cell::FormulaImpl::IsCacheValid() const { return cache_.has_value(); }

void Cell::FormulaImpl::InvalidateCache() const { cache_.reset(); }