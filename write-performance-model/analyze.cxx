// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/RColumnElementBase.hxx>
#include <ROOT/REntry.hxx>
#include <ROOT/RFieldBase.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct RNTupleAnalyzer final {
  std::unique_ptr<ROOT::RNTupleReader> fReader;

  std::uint64_t fNBytes = 0;
  std::uint64_t fNBytesOnStorage = 0;

  std::size_t fNFields = 0;
  std::size_t fNRecordFields = 0;
  std::size_t fNCollectionFields = 0;
  std::size_t fNLeafFields = 0;

  std::size_t fNColumns = 0;

private:
  struct VisitedField {
    std::size_t fNColumnAppends = 0;
    std::optional<ROOT::RNTupleCollectionView> fCollectionView;
    std::vector<VisitedField> fSubfields;

    template <typename Index> std::uint64_t CountColumnAppends(Index index) {
      std::uint64_t columnAppends = fNColumnAppends;

      if (fCollectionView) {
        R__ASSERT(fSubfields.size() == 1);
        auto &itemField = fSubfields[0];
        // Assume the item field is simple if it has no subfields. Additionally
        // require that it has only a single column, which properly treats
        // std::bitset and std::string as non-simple fields.
        const bool isSimpleItemField =
            (itemField.fSubfields.empty() && itemField.fNColumnAppends == 1);

        // For collections, we need to know the range.
        auto range = fCollectionView->GetCollectionRange(index);
        // For simple item fields, ROOT uses AppendV.
        if (isSimpleItemField && range.size() > 0) {
          columnAppends++;
        } else {
          for (auto index : range) {
            columnAppends += itemField.CountColumnAppends(index);
          }
        }
      } else {
        for (auto &field : fSubfields) {
          columnAppends += field.CountColumnAppends(index);
        }
      }

      return columnAppends;
    }
  };
  VisitedField fFieldZero;

public:
  RNTupleAnalyzer(std::string_view ntupleName, std::string_view storage) {
    fReader = ROOT::RNTupleReader::Open(ntupleName, storage);
    VisitStructure();
  }

private:
  void VisitStructure() {
    VisitFields();
    VisitColumns();
  }

  void VisitFields() {
    fFieldZero = VisitField(fReader->GetDescriptor().GetFieldZeroId());
  }

  VisitedField VisitField(ROOT::DescriptorId_t fieldId) {
    const auto &descriptor = GetDescriptor();
    const auto &field = descriptor.GetFieldDescriptor(fieldId);
    VisitedField visitedField;

    if (fieldId != descriptor.GetFieldZeroId()) {
      fNFields++;
      switch (field.GetStructure()) {
      case ROOT::ENTupleStructure::kRecord:
        fNRecordFields++;
        break;
      case ROOT::ENTupleStructure::kCollection:
        fNCollectionFields++;
        visitedField.fNColumnAppends = 1;
        visitedField.fCollectionView = fReader->GetCollectionView(fieldId);
        break;
      case ROOT::ENTupleStructure::kLeaf:
        fNLeafFields++;
        // Assume one append per column; this also works for two columns of
        // std::string.
        // TODO: Think about handling repetitive fields (std::array,
        // std::bitset)
        visitedField.fNColumnAppends = field.GetLogicalColumnIds().size();
        break;
      default:
        break;
      }
    }

    for (const auto &field : descriptor.GetFieldIterable(fieldId)) {
      if (field.IsProjectedField()) {
        continue;
      }
      visitedField.fSubfields.push_back(VisitField(field.GetId()));
    }

    return visitedField;
  }

  void VisitColumns() {
    const auto &descriptor = GetDescriptor();
    for (const auto &column : descriptor.GetColumnIterable()) {
      if (column.IsAliasColumn()) {
        continue;
      }

      fNColumns++;
      auto elementSize =
          ROOT::Internal::RColumnElementBase::Generate(column.GetType())
              ->GetSize();
      for (const auto &cluster : descriptor.GetClusterIterable()) {
        const auto &pageRange = cluster.GetPageRange(column.GetPhysicalId());
        for (const auto &page : pageRange.GetPageInfos()) {
          fNBytes += page.GetNElements() * elementSize;
          fNBytesOnStorage += page.GetLocator().GetNBytesOnStorage();
        }
      }
    }
  }

public:
  std::uint64_t CountColumnAppends() {
    std::uint64_t columnAppends = 0;
    for (auto index : *fReader) {
      columnAppends += CountColumnAppends(index);
    }
    return columnAppends;
  }
  std::uint64_t CountColumnAppends(ROOT::NTupleSize_t index) {
    return fFieldZero.CountColumnAppends(index);
  }

  const ROOT::RNTupleDescriptor &GetDescriptor() const {
    return fReader->GetDescriptor();
  }
};

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./analyze ntupleName storage" << std::endl;
    return 1;
  }

  const char *ntupleName = argv[1];
  const char *storage = argv[2];
  std::cout << "Analyzing \"" << ntupleName << "\" in \"" << storage
            << "\"...\n";

  RNTupleAnalyzer analyzer(ntupleName, storage);
  const auto &descriptor = analyzer.GetDescriptor();
  std::cout << "# Entries: " << descriptor.GetNEntries() << "\n";

  std::cout << "# Bytes: " << analyzer.fNBytes << "\n";
  std::cout << "  On Storage: " << analyzer.fNBytesOnStorage << "\n";

  std::cout << "# Fields: " << descriptor.GetNFields() << "\n";
  std::cout << "  Counted: " << analyzer.fNFields << "\n";
  std::cout << "  Record Fields: " << analyzer.fNRecordFields << "\n";
  std::cout << "  Collection Fields: " << analyzer.fNCollectionFields << "\n";
  std::cout << "  Leaf Fields: " << analyzer.fNLeafFields << "\n";

  std::cout << "# Columns: " << descriptor.GetNPhysicalColumns() << "\n";
  std::cout << "  Counted: " << analyzer.fNColumns << "\n";

  std::cout << "# Column Appends: " << analyzer.CountColumnAppends() << "\n";
}
