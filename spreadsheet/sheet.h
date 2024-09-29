#pragma once

#include "common.h"
#include "cell.h"

#include <functional>
#include <vector>

class Sheet : public SheetInterface
{
    using Table = std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher>;

public:
    Sheet() = default;
    virtual ~Sheet() override;

    void SetCell(Position pos, std::string text) override;

    const CellInterface *GetCell(Position pos) const override;
    CellInterface *GetCell(Position pos) override; 

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream &output) const override;

    void PrintTexts(std::ostream &output) const override;

private:
    // Можете дополнить ваш класс нужными полями и методами
    Table table_;
    Positions positions_;

    bool IsCellAvailable(Position pos) const;
};