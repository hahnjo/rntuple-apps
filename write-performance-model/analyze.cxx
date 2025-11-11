// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/RColumnElementBase.hxx>
#include <ROOT/REntry.hxx>
#include <ROOT/RFieldBase.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>

#include <cstdint>
#include <fstream>
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

  struct DataCounts {
    std::uint64_t fColumnAppends = 0;
    std::uint64_t fCollectionAppends = 0;
    std::uint64_t fEmptyCollectionAppends = 0;
    std::uint64_t fVisitedRecordFields = 0;
    std::uint64_t fVisitedCollectionFields = 0;
    std::uint64_t fVisitedColumns = 0;

    DataCounts &operator+=(const DataCounts &rhs) {
      fColumnAppends += rhs.fColumnAppends;
      fCollectionAppends += rhs.fCollectionAppends;
      fEmptyCollectionAppends += rhs.fEmptyCollectionAppends;
      fVisitedRecordFields += rhs.fVisitedRecordFields;
      fVisitedCollectionFields += rhs.fVisitedCollectionFields;
      fVisitedColumns += rhs.fVisitedColumns;
      return *this;
    }
  };

private:
  struct FieldInfo {
    std::size_t fNColumns = 0;
    std::size_t fNRepetitions = 1;
    std::optional<ROOT::RNTupleCollectionView> fCollectionView;
    std::vector<FieldInfo> fSubfields;
    bool fIsRecordField = false;
    bool fIsCollectionField = false;
    bool fIsSimpleField = false;
    bool fVisited = false;

    DataCounts CountComplexArray(ROOT::NTupleSize_t globalIndex) {
      R__ASSERT(fSubfields.size() == 1);
      auto &itemField = fSubfields[0];

      DataCounts counts;
      for (std::size_t i = 0; i < fNRepetitions; i++) {
        counts += itemField.CountData(globalIndex * fNRepetitions + i);
      }
      return counts;
    }

    DataCounts CountComplexArray(ROOT::RNTupleLocalIndex localIndex) {
      R__ASSERT(fSubfields.size() == 1);
      auto &itemField = fSubfields[0];

      DataCounts counts;
      for (std::size_t i = 0; i < fNRepetitions; i++) {
        counts += itemField.CountData(
            ROOT::RNTupleLocalIndex(localIndex.GetClusterId(),
                                    localIndex.GetIndexInCluster() *
                                        fNRepetitions) +
            i);
      }
      return counts;
    }

    template <typename Index> DataCounts CountData(Index index) {
      fVisited = true;

      DataCounts counts;
      counts.fColumnAppends = fNColumns;

      if (fCollectionView) {
        R__ASSERT(fNRepetitions == 1);
        R__ASSERT(fSubfields.size() == 1);
        auto &itemField = fSubfields[0];

        // For collections, we need to know the range.
        auto range = fCollectionView->GetCollectionRange(index);

        counts.fCollectionAppends++;
        if (range.size() == 0) {
          counts.fEmptyCollectionAppends++;
        }

        // For simple item fields, ROOT uses AppendV.
        if (itemField.fIsSimpleField && range.size() > 0) {
          counts.fColumnAppends++;
        } else {
          for (auto index : range) {
            counts += itemField.CountData(index);
          }
        }
      } else if (fNRepetitions > 1) {
        if (!fSubfields.empty()) {
          // std::array
          R__ASSERT(fNColumns == 0);
          R__ASSERT(fSubfields.size() == 1);
          auto &itemField = fSubfields[0];
          // For simple item fields, ROOT uses AppendV.
          if (itemField.fIsSimpleField) {
            counts.fColumnAppends += 1;
          } else {
            counts += CountComplexArray(index);
          }
        } else {
          // std::bitset - currently not optimized with AppendV.
          R__ASSERT(fNColumns == 1);
          counts.fColumnAppends += fNRepetitions - 1;
        }
      } else {
        for (auto &field : fSubfields) {
          counts += field.CountData(index);
        }
      }

      return counts;
    }

    void CountVisitedAndReset(DataCounts &counts) {
      if (!fVisited) {
        return;
      }

      if (fIsRecordField) {
        counts.fVisitedRecordFields++;
      } else if (fIsCollectionField) {
        counts.fVisitedCollectionFields++;
      }
      counts.fVisitedColumns += fNColumns;
      fVisited = false;
      for (auto &field : fSubfields) {
        field.CountVisitedAndReset(counts);
      }
    }
  };
  FieldInfo fFieldZero;

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

  FieldInfo VisitField(ROOT::DescriptorId_t fieldId) {
    const auto &descriptor = GetDescriptor();
    const auto &field = descriptor.GetFieldDescriptor(fieldId);
    FieldInfo fieldInfo;

    for (const auto &field : descriptor.GetFieldIterable(fieldId)) {
      if (field.IsProjectedField()) {
        continue;
      }
      fieldInfo.fSubfields.push_back(VisitField(field.GetId()));
    }

    if (fieldId != descriptor.GetFieldZeroId()) {
      fNFields++;
      switch (field.GetStructure()) {
      case ROOT::ENTupleStructure::kRecord:
        fNRecordFields++;
        fieldInfo.fIsRecordField = true;
        break;
      case ROOT::ENTupleStructure::kCollection:
        fNCollectionFields++;
        fieldInfo.fIsCollectionField = true;
        fieldInfo.fNColumns = 1;
        fieldInfo.fCollectionView = fReader->GetCollectionView(fieldId);
        break;
      case ROOT::ENTupleStructure::kLeaf: {
        // Assume one append per column; this also works for two columns of
        // std::string.
        fieldInfo.fNColumns = field.GetLogicalColumnIds().size();
        fieldInfo.fNRepetitions = field.GetNRepetitions();
        const std::string &type = field.GetTypeName();
        if (type.rfind("std::array<", 0) == 0 ||
            type.rfind("std::bitset<") == 0 || type == "std::string") {
          // These types are treated as collection fields, even though they have
          // a different strutural role on disk.
          fNCollectionFields++;
          fieldInfo.fIsCollectionField = true;
        } else {
          fNLeafFields++;
          // A field can only be simple if it has no subfields and a single
          // column. Note that this also applies to std::bitset, which we
          // treat explicitly above.
          fieldInfo.fIsSimpleField =
              fieldInfo.fSubfields.empty() && fieldInfo.fNColumns == 1;
        }
        break;
      }
      default:
        break;
      }
    }

    return fieldInfo;
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
  DataCounts CountData() {
    DataCounts counts;
    for (auto index : *fReader) {
      counts += CountData(index);
    }
    return counts;
  }
  DataCounts CountData(ROOT::NTupleSize_t index) {
    DataCounts counts = fFieldZero.CountData(index);
    fFieldZero.CountVisitedAndReset(counts);
    return counts;
  }

  const ROOT::RNTupleDescriptor &GetDescriptor() const {
    return fReader->GetDescriptor();
  }
};

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./analyze ntupleName storage <parameters>"
              << std::endl;
    return 1;
  }

  const char *ntupleName = argv[1];
  const char *storage = argv[2];
  std::cout << "Analyzing \"" << ntupleName << "\" in \"" << storage
            << "\"...\n";

  RNTupleAnalyzer analyzer(ntupleName, storage);
  const auto &descriptor = analyzer.GetDescriptor();
  ROOT::NTupleSize_t entries = descriptor.GetNEntries();
  std::cout << "# Entries: " << entries << "\n";

  std::cout << "# Bytes: " << analyzer.fNBytes << "\n";
  std::cout << "  On Storage: " << analyzer.fNBytesOnStorage << "\n";

  std::cout << "# Fields: " << descriptor.GetNFields() << "\n";
  std::cout << "  Counted: " << analyzer.fNFields << "\n";
  std::cout << "  Record Fields: " << analyzer.fNRecordFields << "\n";
  std::cout << "  Collection Fields: " << analyzer.fNCollectionFields << "\n";
  std::cout << "  Leaf Fields: " << analyzer.fNLeafFields << "\n";

  std::cout << "# Columns: " << descriptor.GetNPhysicalColumns() << "\n";
  std::cout << "  Counted: " << analyzer.fNColumns << "\n";

  std::cout << "\nFrom data:\n";

  auto counts = analyzer.CountData();
  std::uint64_t columnAppends = counts.fColumnAppends;
  std::cout << "# Column Appends: " << columnAppends << "\n";

  std::cout << "# Collection Appends: " << counts.fCollectionAppends;
  std::cout << " (empty: " << counts.fEmptyCollectionAppends << ", ";
  double percent =
      100.0 * counts.fEmptyCollectionAppends / counts.fCollectionAppends;
  std::cout << percent << "%)\n";

  std::cout << "# Visited Record Fields: " << counts.fVisitedRecordFields << " (";
  percent = 100.0 * counts.fVisitedRecordFields / (entries * analyzer.fNRecordFields);
  std::cout << percent << "%)\n";
  std::cout << "# Visited Collection Fields: " << counts.fVisitedCollectionFields << " (";
  percent = 100.0 * counts.fVisitedCollectionFields / (entries * analyzer.fNCollectionFields);
  std::cout << percent << "%)\n";
  std::cout << "# Visited Columns: " << counts.fVisitedColumns << " (";
  percent = 100.0 * counts.fVisitedColumns / (entries * analyzer.fNColumns);
  std::cout << percent << "%)\n";

  if (argc > 3) {
    std::ifstream parameters(argv[3]);
    double perByte, perByteErr;
    parameters >> perByte >> perByteErr;
    double perRecordField, perRecordFieldErr;
    parameters >> perRecordField >> perRecordFieldErr;
    double perCollectionField, perCollectionFieldErr;
    parameters >> perCollectionField >> perCollectionFieldErr;
    double perColumn, perColumnErr;
    parameters >> perColumn >> perColumnErr;
    double perColumnAppend, perColumnAppendErr;
    parameters >> perColumnAppend >> perColumnAppendErr;

    static constexpr double UsToSeconds = 1e-6;
    std::cout << "\n === PERFORMANCE MODEL ===\n";

    double perBytePred = analyzer.fNBytes * perByte * UsToSeconds;
    double perBytePredErr = analyzer.fNBytes * perByteErr * UsToSeconds;
    std::cout << "per byte: " << perByte << " us +- " << perByteErr << " us\n";
    std::cout << " -> prediction: " << perBytePred << " s +- " << perBytePredErr
              << " s\n";

    double perRecordFieldPred =
        entries * analyzer.fNRecordFields * perRecordField * UsToSeconds;
    double perRecordFieldPredErr =
        entries * analyzer.fNRecordFields * perRecordFieldErr * UsToSeconds;
    double perRecordFieldVisitedPred =
        counts.fVisitedRecordFields * perRecordField * UsToSeconds;
    double perRecordFieldVisitedPredErr =
        counts.fVisitedRecordFields * perRecordFieldErr * UsToSeconds;
    std::cout << "per record field: " << perRecordField << " us +- "
              << perRecordFieldErr << " us\n";
    std::cout << " -> prediction: " << perRecordFieldPred << " s +- "
              << perRecordFieldPredErr << " s\n";
    std::cout << " -> w/ visited: " << perRecordFieldVisitedPred << " s +- "
              << perRecordFieldVisitedPredErr << " s\n";

    double perCollectionFieldPred = entries * analyzer.fNCollectionFields *
                                    perCollectionField * UsToSeconds;
    double perCollectionFieldPredErr = entries * analyzer.fNCollectionFields *
                                       perCollectionFieldErr * UsToSeconds;
    double perCollectionFieldVisitedPred =
        counts.fVisitedCollectionFields * perCollectionField * UsToSeconds;
    double perCollectionFieldVisitedPredErr =
        counts.fVisitedCollectionFields * perCollectionFieldErr * UsToSeconds;
    std::cout << "per collection field: " << perCollectionField << " us +- "
              << perCollectionFieldErr << " us\n";
    std::cout << " -> prediction: " << perCollectionFieldPred << " s +- "
              << perCollectionFieldPredErr << " s\n";
    std::cout << " -> w/ visited: " << perCollectionFieldVisitedPred << " s +- "
              << perCollectionFieldVisitedPredErr << " s\n";

    double perColumnPred =
        entries * analyzer.fNColumns * perColumn * UsToSeconds;
    double perColumnPredErr =
        entries * analyzer.fNColumns * perColumnErr * UsToSeconds;
    double perColumnVisitedPred =
        counts.fVisitedColumns * perColumn * UsToSeconds;
    double perColumnVisitedPredErr =
        counts.fVisitedColumns * perColumnErr * UsToSeconds;
    std::cout << "per column: " << perColumn << " us +- " << perColumnErr
              << " us\n";
    std::cout << " -> prediction: " << perColumnPred << " s +- "
              << perColumnPredErr << " s\n";
    std::cout << " -> w/ visited: " << perColumnVisitedPred << " s +- "
              << perColumnVisitedPredErr << " s\n";

    double perColumnAppendPred = columnAppends * perColumnAppend * UsToSeconds;
    double perColumnAppendPredErr =
        columnAppends * perColumnAppendErr * UsToSeconds;
    std::cout << "per column append: " << perColumnAppend << " us +- "
              << perColumnAppendErr << " us\n";
    std::cout << " -> prediction: " << perColumnAppendPred << " s +- "
              << perColumnAppendPredErr << " s\n";

    double totalPred = perBytePred + perRecordFieldPred +
                       perCollectionFieldPred + perColumnPred +
                       perColumnAppendPred;
    double totalPredErr = perBytePredErr + perRecordFieldPredErr +
                          perCollectionFieldPredErr + perColumnPredErr +
                          perColumnAppendPredErr;
    double totalVisitedPred = perBytePred + perRecordFieldVisitedPred +
                              perCollectionFieldVisitedPred +
                              perColumnVisitedPred + perColumnAppendPred;
    double totalVisitedPredErr = perBytePredErr + perRecordFieldVisitedPredErr +
                                 perCollectionFieldVisitedPredErr +
                                 perColumnVisitedPredErr +
                                 perColumnAppendPredErr;
    std::cout << "\n => TOTAL PREDICTION: " << totalPred << " s +- "
              << totalPredErr << " s\n";
    std::cout << " => based on visited: " << totalVisitedPred << " s +- "
              << totalVisitedPredErr << " s\n";
  }

  return 0;
}
