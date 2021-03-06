/*
  Utilities for AFF4 imaging. These are mostly high level utilities used by the
  command line imager.
*/
#include "libaff4.h"
#include "aff4_imager_utils.h"
#include <iostream>
#include <time.h>

using std::cout;

AFF4Status ImageStream(DataStore &resolver, vector<URN> &input_urns,
                       URN output_urn,
                       bool truncate,
                       size_t buffer_size) {
  AFF4Status result = STATUS_OK;

  // We are allowed to write on the output file.
  if(truncate) {
    LOG(INFO) << "Truncating output file: " << output_urn.value.c_str();
    resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE, new XSDString("truncate"));
  } else {
    resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE, new XSDString("append"));
  };

  AFF4ScopedPtr<AFF4Stream> output = resolver.AFF4FactoryOpen<AFF4Stream>(
      output_urn);

  if(!output) {
    LOG(ERROR) << "Failed to create output file: " << output_urn.value.c_str()
               << ".\n";
    return IO_ERROR;
  };

  AFF4ScopedPtr<ZipFile> zip = ZipFile::NewZipFile(&resolver, output->urn);
  if(!zip) {
    return IO_ERROR;
  };

  for(URN input_urn: input_urns) {
    cout << "Adding " << input_urn.value.c_str() << "\n";

    AFF4ScopedPtr<AFF4Stream> input = resolver.AFF4FactoryOpen<AFF4Stream>(
        input_urn);

    if(!input) {
      LOG(ERROR) << "Failed to open input file: " << input_urn.value.c_str()
                 << ".\n";
      result = IO_ERROR;
      continue;
    };

    // Create a new image in this volume.
    URN image_urn = zip->urn.Append(input_urn.Parse().path);

    AFF4ScopedPtr<AFF4Image> image = AFF4Image::NewAFF4Image(
        &resolver, image_urn, zip->urn);

    if(!image) {
      return IO_ERROR;
    };

    input->CopyToStream(*image, input->Size());
  };

  return result;
};


AFF4Status ExtractStream(DataStore &resolver, URN input_urn,
                         URN output_urn,
                         bool truncate,
                         size_t buffer_size) {
  // We are allowed to write on the output file.
  if(truncate) {
    LOG(INFO) << "Truncating output file: " << output_urn.value.c_str();
    resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE, new XSDString("truncate"));
  } else {
    resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE, new XSDString("append"));
  };

  AFF4ScopedPtr<AFF4Stream> input = resolver.AFF4FactoryOpen<AFF4Stream>(
      input_urn);
  AFF4ScopedPtr<AFF4Stream> output = resolver.AFF4FactoryOpen<AFF4Stream>(
      output_urn);

  if(!input) {
    LOG(ERROR) << "Failed to open input stream. " << input_urn.value.c_str()
               << ".\n";
    return IO_ERROR;
  };

  if(!output) {
    LOG(ERROR) << "Failed to create output file: " << output_urn.value.c_str()
               << ".\n";
    return IO_ERROR;
  };

  return input->CopyToStream(*output, input->Size());
  return STATUS_OK;
};


AFF4Status BasicImager::ParseArgs(int argc, char** argv)  {
  AFF4Status res = Initialize();
  if(res != STATUS_OK)
    return res;

  RegisterArgs();

  TCLAP::CmdLine cmd(GetName(), ' ', GetVersion());

  for(auto it=args.rbegin(); it != args.rend(); it++) {
    cmd.add(it->get());
  };

  try {
    cmd.parse(argc,argv);
    return HandlerDispatch();

  } catch (TCLAP::ArgException& e) {
    LOG(ERROR) << e.error() << " " << e.argId();
    return GENERIC_ERROR;
  }
};

AFF4Status BasicImager::HandlerDispatch() {
  AFF4Status result = CONTINUE;

  // Check for incompatible commands.
  if(Get("export")->isSet() && Get("input")->isSet()) {
    std::cout << "--export and --input are incompatible. "
        "Please select only one.\n";
    return INCOMPATIBLE_TYPES;
  };

  if(result==CONTINUE && Get("compression")->isSet())
    result = handle_compression();

  if(result==CONTINUE && Get("verbose")->isSet())
    result = handle_Verbose();

  if(result==CONTINUE && Get("aff4_volumes")->isSet())
    result = handle_aff4_volumes();

  if(result==CONTINUE && Get("view")->isSet())
    result = handle_view();

  if(result==CONTINUE && Get("input")->isSet())
    result = handle_input();

  if(result==CONTINUE && Get("export")->isSet())
    result = handle_export();

  return result;
};


AFF4Status BasicImager::handle_Verbose() {
  google::SetStderrLogging(google::GLOG_INFO);

  return CONTINUE;
};

AFF4Status BasicImager::handle_aff4_volumes() {
  vector<string> v = GetArg<TCLAP::UnlabeledMultiArg<string>>(
      "aff4_volumes")->getValue();

  for (unsigned int i = 0; i < v.size(); i++) {
    LOG(INFO) << "Preloading AFF4 Volume: " << v[i];
    AFF4ScopedPtr<ZipFile> zip = ZipFile::NewZipFile(&resolver, v[i]);
    if(zip->members.size() == 0) {
      LOG(ERROR) << "Unable to load " << v[i]
                 << " as an existing AFF4 Volume.";
      return IO_ERROR;
    };

    volume_URN = zip->urn;
  };

  return CONTINUE;
};

AFF4Status BasicImager::handle_view() {
  resolver.Dump(GetArg<TCLAP::SwitchArg>("verbose")->getValue());
  return STATUS_OK;
}

AFF4Status BasicImager::handle_input() {
  // Get the output volume.
  URN volume_urn;
  AFF4Status res = GetOutputVolumeURN(volume_urn);
  if(res != STATUS_OK)
    return res;

  vector<string> inputs = GetArg<TCLAP::MultiArgToNextFlag<string>>(
      "input")->getValue();

  res = CONTINUE;
  for(string input: inputs) {
    URN input_urn(URN::NewURNFromFilename(input));

    std::cout << "Adding " << input.c_str() << "\n";

    // Try to open the input.
    AFF4ScopedPtr<AFF4Stream> input_stream = resolver.AFF4FactoryOpen<
      AFF4Stream>(input_urn);

    // Not valid - skip it.
    if(!input_stream) {
      res = IO_ERROR;
      continue;
    };

    // Create a new AFF4Image in this volume.
    URN image_urn = volume_urn.Append(input);

    AFF4ScopedPtr<AFF4Image> image_stream = AFF4Image::NewAFF4Image(
        &resolver, image_urn, volume_urn);

    // Cant write to the output stream at all, this is considered fatal.
    if(!image_stream) {
      return IO_ERROR;
    };

    // Set the output compression according to the user's wishes.
    image_stream->compression = compression;

    // Copy the input stream to the output stream.
    input_stream->CopyToStream(*image_stream, input_stream->Size());
  };

  return res;
}

AFF4Status BasicImager::handle_export() {
  if(!Get("output")->isSet()) {
    cout << "ERROR: Can not specify an export without an output\n";
    return INVALID_INPUT;
  };

  string output = GetArg<TCLAP::ValueArg<string>>("output")->getValue();
  string export_ = GetArg<TCLAP::ValueArg<string>>("export")->getValue();
  URN export_urn(export_);
  URN output_urn(output);

  // We do not want to interpret this parameter as a file reference since it
  // must come from the image.
  if (volume_URN.value.size() > 0 &&
      export_urn.Scheme() == "file") {
    LOG(INFO) << "Interpreting export URN as relative to volume " <<
        volume_URN.value;

    export_urn = volume_URN.Append(export_);
  };

  // When we export we always truncate the output file.
  resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE,
               new XSDString("truncate"));

  cout << "Extracting " << export_urn.value << " into " <<
      output_urn.value << "\n";
  AFF4Status res = ExtractStream(
      resolver, export_urn, output_urn, Get("truncate")->isSet());
  if(res == STATUS_OK)
    return CONTINUE;

  return res;
};


AFF4Status BasicImager::GetOutputVolumeURN(URN &volume_urn) {
  if(output_volume_urn.value.size() > 0) {
    volume_urn = output_volume_urn;
    return STATUS_OK;
  };

  if(!Get("output")->isSet())
    return INVALID_INPUT;

  string output_path = GetArg<TCLAP::ValueArg<string>>("output")->getValue();
  URN output_urn(URN::NewURNFromFilename(output_path));

  // We are allowed to write on the output file.
  if(Get("truncate")->isSet()) {
    LOG(INFO) << "Truncating output file: " << output_urn.SerializeToString();

    resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE, new XSDString("truncate"));
  } else {
    resolver.Set(output_urn, AFF4_STREAM_WRITE_MODE, new XSDString("append"));
  };

  AFF4ScopedPtr<AFF4Stream> output_stream = resolver.AFF4FactoryOpen<AFF4Stream>(
      output_urn);

  if(!output_stream) {
    LOG(ERROR) << "Failed to create output file: " <<
        output_urn.SerializeToString();

    return IO_ERROR;
  };

  AFF4ScopedPtr<ZipFile> zip = ZipFile::NewZipFile(
      &resolver, output_stream->urn);

  if(!zip) {
    return IO_ERROR;
  };

  volume_urn = zip->urn;
  output_volume_urn = zip->urn;

  return STATUS_OK;
};


AFF4Status BasicImager::handle_compression() {
  string compression_setting = GetArg<TCLAP::ValueArg<string>>(
      "compression")->getValue();

  if(compression_setting == "zlib") {
    compression = AFF4_IMAGE_COMPRESSION_ENUM_ZLIB;
  } else if(compression_setting == "snappy") {
    compression = AFF4_IMAGE_COMPRESSION_ENUM_SNAPPY;

  } else if(compression_setting == "none") {
    compression = AFF4_IMAGE_COMPRESSION_ENUM_STORED;

  } else {
    LOG(ERROR) << "Unknown compression scheme " << compression;
    return INVALID_INPUT;
  };

  std::cout << "Setting compression " << compression_setting.c_str() << "\n";

  return CONTINUE;
};
