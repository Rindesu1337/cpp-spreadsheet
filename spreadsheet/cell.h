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
  Cell(SheetInterface &sheet, Position pos);
  Cell(SheetInterface &sheet, Position pos, std::string text);
  virtual ~Cell() override = default;

  void Set(std::string text);
  void Clear();
  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;
  bool IsReferenced() const;
  void ClearCache();

private:
  // можете воспользоваться нашей подсказкой, но это необязательно.
  /*    class Impl;
      class EmptyImpl;
      class TextImpl;
      class FormulaImpl;
      std::unique_ptr<Impl> impl_;
  */
  class Impl {
  public:
    virtual ~Impl() = default;

    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
  };

  class FormulaImpl : public Impl {
  public:
    FormulaImpl(std::string text, const SheetInterface &sheet);
    virtual ~FormulaImpl() override = default;

    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

  private:
    const SheetInterface &sheet_;
    std::unique_ptr<FormulaInterface> value_;
  };

  class EmptyImpl : public Impl {
  public:
    EmptyImpl() = default;
    virtual ~EmptyImpl() override = default;

    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
  };

  class TextImpl : public Impl {
  public:
    TextImpl(std::string text);
    virtual ~TextImpl() override = default;

    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

  private:
    std::string value_;
  };

private:
  bool CheckCyclicality(
      std::unique_ptr<Impl> &impl) const; // Проверка циклической зависимости
  bool IsCyclic(const Positions &dependents, Positions &viewed) const;

  void RemoveOldDependents();
  void AddNewDependents(const Positions &new_dependents);
  void AddReferencedCells(const std::vector<Position> &new_refs);
  void ResetCacheDependents();

private:
  SheetInterface &sheet_;

  Position pos_;
  std::unique_ptr<Impl> impl_; // Значение ячейки таблицы

  Positions dependents_; // кто ссылается на ячейку std::unordered_set<Position,
                         // PositionHasher>
  Positions includes_; // на кого указывает ячейка  std::unordered_set<Position,
                       // PositionHasher>

  mutable std::optional<Value> cache_;
};