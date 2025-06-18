#include <carpeDM.h>
#include <validation.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")("file", po::value<std::string>(), "dot file to validate");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("file")) {
    std::cout << "File to validate: " << vm["file"].as<std::string>() << "\n";
  } else {
    std::cout << "No file to validate specified.\n";
    exit(-1);
  }

  CarpeDM cdm;
  auto fileText = cdm.readTextFile(vm.at("file").as<std::string>());

  Graph g;
  cdm.parseDot(fileText, g);

  Validation::init();
  try {
    BOOST_FOREACH (vertex_t v, vertices(g)) {
      Validation::neighbourhoodCheck(v, g);
      Validation::neighbourhoodCheckCpu(v, g);
      Validation::eventSequenceCheck(v, g, true);
    }
  } catch (std::runtime_error const& err) {
    std::cerr << "Validation failed: " << err.what() << std::endl;
    return -1;
  }
  std::cout << "Validation successful." << std::endl;
}