#include <iostream>
#include <string>
#include <cstring>

#include "bench.hpp"
#include "algorithms/hnsw.hpp"

const char* get_arg_value(int argc, char* argv[], const char* flag, int& i) {
  if (i + 1 < argc && argv[i + 1][0] != '-') {
      return argv[++i];
  }
  std::cerr << "Missing " << flag << std::endl;
  exit(1);
}

int main(int argc, char* argv[]) {
    std::string data_type, data_path, query_file, res_path;
    size_t begin_num = 5000, batch_size = 100, max_elements = 1000000;
    float write_ratio = 0.5;
    size_t recall_at = 10, R = 16, Ls = 50, dim = 128, num_threads = std::thread::hardware_concurrency();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--data_type") == 0) data_path = get_arg_value(argc, argv, "--data_type", i);
        else if (strcmp(argv[i], "--data_path") == 0) data_path = get_arg_value(argc, argv, "--data_path", i);
        else if (strcmp(argv[i], "--query_file") == 0) query_file = get_arg_value(argc, argv, "--query_file", i);
        else if (strcmp(argv[i], "--begin_num") == 0) begin_num = std::stoul(get_arg_value(argc, argv, "--begin_num", i));
        else if (strcmp(argv[i], "--max_elements") == 0) begin_num = std::stoul(get_arg_value(argc, argv, "--max_elements", i));
        else if (strcmp(argv[i], "--write_ratio") == 0) write_ratio = std::stof(get_arg_value(argc, argv, "--write_ratio", i));
        else if (strcmp(argv[i], "--batch_size") == 0) batch_size = std::stoul(get_arg_value(argc, argv, "--batch_size", i));
        else if (strcmp(argv[i], "--recall_at") == 0) recall_at = std::stoul(get_arg_value(argc, argv, "--recall_at", i));
        else if (strcmp(argv[i], "--R") == 0) Ls = std::stoul(get_arg_value(argc, argv, "--R", i));
        else if (strcmp(argv[i], "--Ls") == 0) Ls = std::stoul(get_arg_value(argc, argv, "--Ls", i));
        else if (strcmp(argv[i], "--dim") == 0) Ls = std::stoul(get_arg_value(argc, argv, "--dim", i));
        else if (strcmp(argv[i], "--num_threads") == 0) num_threads = std::stoul(get_arg_value(argc, argv, "--num_threads", i));
        else if (strcmp(argv[i], "--res_path") == 0) res_path = get_arg_value(argc, argv, "--res_path", i);
        else {
            std::cerr << "Unkown: " << argv[i] << "\n";
            return 1;
        }
    }

    if (data_path.empty() || query_file.empty() || res_path.empty()) {
        std::cerr << "Need --data_path, --query_file and --res_path\n";
        return 1;
    }

    using TagT = uint32_t;
    using LabelT = uint32_t;
    std::vector<SearchResult<TagT>> search_results;

    if (data_type == "float") {
        using IndexType = IndexBase<float, TagT, LabelT>;
        // using IndexType = HNSW<float, TagT, LabelT>;
        std::unique_ptr<IndexBase<float, TagT, LabelT>> index(new IndexType(dim, max_elements, R, Ls));
        measure_performance([&]() {
            concurrent_bench<float, TagT, LabelT>(data_path, query_file, begin_num, write_ratio, 
                                                    batch_size, recall_at, Ls, num_threads, 
                                                    std::move(index), res_path, search_results);
        }, true);
    } else if (data_type == "int8_t") {
        using IndexType = IndexBase<int8_t, TagT, LabelT>;
        std::unique_ptr<IndexType> index(new IndexType());
        measure_performance([&]() {
            concurrent_bench<int8_t, TagT, LabelT>(data_path, query_file, begin_num, write_ratio, 
                                                    batch_size, recall_at, Ls, num_threads, 
                                                    std::move(index), res_path, search_results);
        }, true);
    } else if (data_type == "uint8_t") {
        using IndexType = IndexBase<uint8_t, TagT, LabelT>;
        std::unique_ptr<IndexType> index(new IndexType());
        measure_performance([&]() {
            concurrent_bench<uint8_t, TagT, LabelT>(data_path, query_file, begin_num, write_ratio, 
                                                    batch_size, recall_at, Ls, num_threads, 
                                                    std::move(index), res_path, search_results);
        }, true);
    } else {
        std::cerr << "Unknown data type: " << data_type << "\n";
        return 1;
    }

    write_results(search_results, res_path);

    return 0;
}