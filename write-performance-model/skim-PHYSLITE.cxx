// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/REntry.hxx>
#include <ROOT/RFieldBase.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>

#include <iostream>
#include <memory>
#include <string>
#include <string_view>

static constexpr const char *kNTupleName = "CollectionTree";

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: ./skim-PHYSLITE source destination\n");
    return 1;
  }

  const char *sourceFileName = argv[1];
  const char *destFileName = argv[2];

  auto model = ROOT::RNTupleModel::CreateBare();
  {
    // Open the source file a first time to create the (skimmed) model.
    auto reader = ROOT::RNTupleReader::Open(kNTupleName, sourceFileName);
    const auto &descriptor = reader->GetDescriptor();
    for (const auto &field : descriptor.GetTopLevelFields()) {
      const auto &typeName = field.GetTypeName();
      // Skip fields that need dictionaries.
      if (typeName.find("xAOD") != std::string::npos) {
        continue;
      } else if (typeName.find("DataVector") != std::string::npos) {
        continue;
      }
      model->AddField(field.CreateField(descriptor));
    }
  }

  // Create reader (with imposed model) and writer.
  auto reader =
      ROOT::RNTupleReader::Open(model->Clone(), kNTupleName, sourceFileName);
  ROOT::RNTupleWriteOptions options;
  options.SetCompression(0);
  auto writer = ROOT::RNTupleWriter::Recreate(model->Clone(), kNTupleName,
                                              destFileName, options);

  // Create entries and link their shared_ptr's.
  auto readerEntry = reader->CreateEntry();
  auto writerEntry = writer->GetModel().CreateBareEntry();
  for (const auto &value : *readerEntry) {
    writerEntry->BindValue(value.GetField().GetFieldName(),
                           value.GetPtr<void>());
  }

  // Iterate over all entries, reading and writing.
  for (auto index : *reader) {
    reader->LoadEntry(index, *readerEntry);
    writer->Fill(*writerEntry);
  }

  return 0;
}
