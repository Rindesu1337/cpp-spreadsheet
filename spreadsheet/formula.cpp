#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
  return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
  // Реализуйте следующие методы:
  explicit Formula(std::string expression) try
      : ast_(ParseFormulaAST(expression)) {
  } catch (const FormulaException &error) {
    throw error;
  }

  Value Evaluate(const SheetInterface &sheet) const override {
    std::function<double(Position)> args = [&sheet](Position pos) {
      if (!sheet.GetCell(pos)) {
        return 0.0;
      }
      auto value = sheet.GetCell(pos)->GetValue();
      if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
      } else if (std::holds_alternative<std::string>(value)) {
        std::string text = sheet.GetCell(pos)->GetText();

        if (text.empty()) {
          return 0.0;
        }
        if (text.front() == ESCAPE_SIGN) {
          throw FormulaError(FormulaError::Category::Value);
        }

        try {
          return std::stod(text);
        } catch (...) {
          throw FormulaError(FormulaError::Category::Value);
        }
      } else {
        throw std::get<FormulaError>(value);
      }
    };

    try {
      return ast_.Execute(args);
    } catch (const FormulaError &fe) {
      return fe;
    }
  }

  std::string GetExpression() const override {
    std::ostringstream out;

    ast_.PrintFormula(out);

    return out.str();
  }

  std::vector<Position> GetReferencedCells() const override {
    std::vector<Position> result;
    std::set<Position> set_of_pos;
    for (const auto &cell : ast_.GetCells()) {
      if (cell.IsValid()) {
        set_of_pos.insert(cell);
      } else {
        continue;
      }
    }
    for (const auto &cell : set_of_pos) {
      result.push_back(cell);
    }
    return result;
  }

private:
  FormulaAST ast_;
};
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
  return std::make_unique<Formula>(std::move(expression));
}