#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

class Sheet;

class Cell : public CellInterface {
public:
  Cell(Sheet &sheet, Position Position);

  virtual ~Cell() override = default;

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;

  std::vector<Position> GetReferencedCells() const override;

private:
  // можете воспользоваться нашей подсказкой, но это необязательно.
  class Impl {
  public:
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual bool IsCacheValid() const;
    virtual void InvalidateCache() const;
  };

  class TextImpl final : public Impl {
  public:
    TextImpl(std::string text);

    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

  private:
    std::string text_;
  };

  class FormulaImpl final : public Impl {
  public:
    FormulaImpl(std::string text, const SheetInterface &sheet);

    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    virtual bool IsCacheValid() const override;
    virtual void InvalidateCache() const override;

  private:
    const SheetInterface &sheet_;
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<FormulaInterface::Value> cache_;
  };

  class EmptyImpl final : public Impl {
  public:
    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
  };

  Sheet &sheet_;

  std::unique_ptr<Impl> impl_; // Значение ячейки таблицы
  std::unordered_set<Cell *> dependent_cells_;
  std::unordered_set<Cell *> referenced_cells_;

  void NewReference(const std::vector<Position> &new_dependents);
  void InvalidCache();
  bool CircularDependency(std::unique_ptr<Cell::Impl> &impl)
      const; // Проверка циклической зависимости
  bool DFS(const PositionsSet &dependents, PositionsSet &verified) const;

  Position position_;
};