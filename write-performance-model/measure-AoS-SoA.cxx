// SPDX-License-Identifier: GPL-3.0-or-later

#include "AoS-SoA.hxx"

#include <ROOT/RColumn.hxx>
#include <ROOT/RColumnElementBase.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleUtil.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageNullSink.hxx>
#include <TROOT.h>
#include <TSystem.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

static constexpr std::size_t NumRepetitions = 10;
static constexpr std::size_t NumFields = 100;
static constexpr std::size_t NumEntries = 100000;
static constexpr std::size_t MinElements = 1;
static constexpr std::size_t MaxElements = 13;

// WARNING: Users should usually not implement custom fields inheriting from
// RFieldBase. This class is a prototype implementation to evaluate the native
// support of SoA data types in RNTuple. It should not be copied or even used
// in production.
class SoAField final : public ROOT::RFieldBase {
  static constexpr std::size_t NumStructFields = 5;
  ROOT::RFieldBase *fStructFields[NumStructFields];
  ROOT::Internal::RColumnIndex fNWritten{0};

public:
  SoAField(std::string_view name)
      : RFieldBase(name, "SoA", ROOT::ENTupleStructure::kCollection, false) {
    auto itemField = std::make_unique<ROOT::RField<S>>("_0");
    std::size_t i = 0;
    for (auto &subfield : *itemField) {
      R__ASSERT(i < NumStructFields);
      fStructFields[i] = &subfield;
      i++;
    }
    R__ASSERT(i == NumStructFields);
    Attach(std::move(itemField));
  }

  std::size_t GetValueSize() const final { return sizeof(SoA); }
  std::size_t GetAlignment() const final { return alignof(SoA); }

protected:
  std::unique_ptr<RFieldBase> CloneImpl(std::string_view newName) const final {
    return std::make_unique<SoAField>(newName);
  }

  void ConstructValue(void *where) const final { new (where) SoA; }

  const RColumnRepresentations &GetColumnRepresentations() const final {
    static RColumnRepresentations representations(
        {{ROOT::ENTupleColumnType::kSplitIndex64},
         {ROOT::ENTupleColumnType::kIndex64},
         {ROOT::ENTupleColumnType::kSplitIndex32},
         {ROOT::ENTupleColumnType::kIndex32}},
        {});
    return representations;
  }

  void GenerateColumns() final {
    GenerateColumnsImpl<ROOT::Internal::RColumnIndex>();
  }
  void GenerateColumns(const ROOT::RNTupleDescriptor &) final {
    throw ROOT::RException(R__FAIL("reading not supported (yet)"));
  }

  std::size_t AppendImpl(const void *from) final {
    // WARNING: Users should usually not implement custom fields and Append
    // methods. This implementation is part of the prototype class to evaluate
    // the native support of SoA data types in RNTuple. It should not be copied
    // or even used in production.

    auto soa = static_cast<const SoA *>(from);
    const std::size_t count = soa->f1.size();
    R__ASSERT(soa->f2.size() == count);
    R__ASSERT(soa->f3.size() == count);
    R__ASSERT(soa->f4.size() == count);
    R__ASSERT(soa->f5.size() == count);

    std::size_t nbytes = 0;
    if (count > 0) {
      GetPrincipalColumnOf(*fStructFields[0])->AppendV(soa->f1.data(), count);
      nbytes += count * GetPrincipalColumnOf(*fStructFields[0])
                            ->GetElement()
                            ->GetPackedSize();
      GetPrincipalColumnOf(*fStructFields[1])->AppendV(soa->f2.data(), count);
      nbytes += count * GetPrincipalColumnOf(*fStructFields[1])
                            ->GetElement()
                            ->GetPackedSize();
      GetPrincipalColumnOf(*fStructFields[2])->AppendV(soa->f3.data(), count);
      nbytes += count * GetPrincipalColumnOf(*fStructFields[2])
                            ->GetElement()
                            ->GetPackedSize();
      GetPrincipalColumnOf(*fStructFields[3])->AppendV(soa->f4.data(), count);
      nbytes += count * GetPrincipalColumnOf(*fStructFields[3])
                            ->GetElement()
                            ->GetPackedSize();
      GetPrincipalColumnOf(*fStructFields[4])->AppendV(soa->f5.data(), count);
      nbytes += count * GetPrincipalColumnOf(*fStructFields[4])
                            ->GetElement()
                            ->GetPackedSize();
    }

    fNWritten += count;
    fPrincipalColumn->Append(&fNWritten);
    return nbytes + fPrincipalColumn->GetElement()->GetPackedSize();
  }

  void CommitClusterImpl() final { fNWritten = 0; }
};

template <typename FieldType, typename ValueType>
double Run(std::function<void(ValueType &)> mod) {
  auto model = ROOT::RNTupleModel::CreateBare();
  for (std::size_t f = 0; f < NumFields; f++) {
    model->AddField(std::make_unique<FieldType>("f" + std::to_string(f)));
  }

  // Create the writer.
  ROOT::RNTupleWriteOptions options;
  options.SetCompression(0);
  options.SetEnableSamePageMerging(false);
  options.SetEnablePageChecksums(false);

  auto sink = std::make_unique<ROOT::Experimental::Internal::RPageNullSink>(
      "null", options);
  auto writer =
      ROOT::Internal::CreateRNTupleWriter(std::move(model), std::move(sink));

  // Prepare entry.
  auto entry = writer->CreateEntry();

  for (std::size_t f = 0; f < NumFields; f++) {
    auto ptr = entry->GetPtr<ValueType>("f" + std::to_string(f));
    mod(*ptr);
  }

  // Fill entries, serializing the data.
  auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < NumEntries; i++) {
    writer->Fill(*entry);
  }

  // Destruct the writer and commit the dataset.
  writer.reset();

  auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> duration = end - start;

  return duration.count();
}

template <typename FieldType, typename ValueType>
void Benchmark(std::function<void(ValueType &)> mod) {
  double sum = 0, sum2 = 0;
  std::cout << "   ";
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = Run<FieldType, ValueType>(mod);
    std::cout << " " << timing << std::flush;
    sum += timing;
    sum2 += timing * timing;
  }
  double mean = sum / NumRepetitions;
  std::cout << "\n     -> " << mean << " s";
  if (NumRepetitions > 1) {
    double var = (sum2 - sum * sum / NumRepetitions) / (NumRepetitions - 1);
    double stdev = std::sqrt(var);
    std::cout << " +- " << stdev << " s";
  }
  std::cout << "\n";
}

int main(int argc, char *argv[]) {
  std::cout << "NumRepetitions: " << NumRepetitions
            << ", NumFields: " << NumFields << ", NumEntries: " << NumEntries
            << "\n\n";

  // Initialize ROOT before starting any benchmark.
  ROOT::GetROOT();

  gSystem->Load("./libAoS-SoA.so");

  std::cout << "Benchmarking AoS..." << std::endl;
  for (std::size_t elements = MinElements; elements <= MaxElements;
       elements++) {
    std::cout << "  " << elements << " element(s):" << std::endl;
    auto mod = [elements](AoS &aos) { aos.resize(elements); };
    Benchmark<ROOT::RField<AoS>, AoS>(mod);
  }
  std::cout << "\n";

  std::cout << "Benchmarking SoA..." << std::endl;
  for (std::size_t elements = MinElements; elements <= MaxElements;
       elements++) {
    std::cout << "  " << elements << " element(s):" << std::endl;
    auto mod = [elements](SoA &soa) {
      soa.f1.resize(elements);
      soa.f2.resize(elements);
      soa.f3.resize(elements);
      soa.f4.resize(elements);
      soa.f5.resize(elements);
    };
    Benchmark<ROOT::RField<SoA>, SoA>(mod);
  }
  std::cout << "\n";

  std::cout << "Benchmarking native SoA..." << std::endl;
  for (std::size_t elements = MinElements; elements <= MaxElements;
       elements++) {
    std::cout << "  " << elements << " element(s):" << std::endl;
    auto mod = [elements](SoA &soa) {
      soa.f1.resize(elements);
      soa.f2.resize(elements);
      soa.f3.resize(elements);
      soa.f4.resize(elements);
      soa.f5.resize(elements);
    };
    Benchmark<SoAField, SoA>(mod);
  }

  return 0;
}
